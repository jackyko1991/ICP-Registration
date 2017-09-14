/**
Author: QIN Shuo
Date: 2017/05/11
*/


#include "ICPRegistrationFilter.h"

ICPRegistrationFilter::ICPRegistrationFilter(QObject *parent) :
AbstractFilter(parent)
{
    m_outputMatrix = vtkMatrix4x4::New();
	m_initialMatrix = vtkMatrix4x4::New();
	m_initialMatrix->Identity();
	m_outputData = vtkPolyData::New();
}

ICPRegistrationFilter::~ICPRegistrationFilter()
{
    m_outputMatrix->Delete();
	m_initialMatrix->Delete();
	m_outputData->Delete();
}

void ICPRegistrationFilter::SetSourcePolyData(vtkPolyData * polydata)
{
	m_sourceData = polydata;
}

void ICPRegistrationFilter::SetTargetPolyData(vtkPolyData * polydata)
{
	m_targetData = polydata;
}

void ICPRegistrationFilter::SetInitialMatrix(vtkMatrix4x4 *matrix)
{
    m_initialMatrix->DeepCopy(matrix);
}

void ICPRegistrationFilter::AppendWorldCoordiantion(double* w_coord)
{
    m_worldCoordinationList.append(w_coord);
}

void ICPRegistrationFilter::ClearAllPoints()
{
    m_worldCoordinationList.clear();
}

double ICPRegistrationFilter::GetOutputError()
{
    return m_outputError;
}

vtkMatrix4x4 *ICPRegistrationFilter::GetOutputMatrix()
{
    return m_outputMatrix;
}

vtkPolyData * ICPRegistrationFilter::GetOutputData()
{
	return m_outputData;
}

void ICPRegistrationFilter::GetOutputData(vtkPolyData *polydata)
{
	polydata->DeepCopy(m_outputData);
}

/*
Procedure:
source  -- step1  -->   coarse transformed points   -- step2 ->  fine transformed points
        coarse matrix                                fine matrix  

How to get coarse transformed points: use vtkTrackerUtil, set coarse matrix to vtkTrackerUtil and get points
*/
void ICPRegistrationFilter::Compute()
{
    //Add ICP Registration function here
    //with use of m_initialMatrix and polydata

	// Setup ICP transform
	vtkSmartPointer<vtkIterativeClosestPointTransform> icp =
		vtkSmartPointer<vtkIterativeClosestPointTransform>::New();
	icp->SetSource(m_sourceData);
	icp->SetTarget(m_targetData);
	icp->GetLandmarkTransform()->SetModeToRigidBody();
	icp->SetMaximumNumberOfIterations(20);
	//icp->StartByMatchingCentroidsOn();
	icp->Modified();
	icp->Update();

	// Get the resulting transformation matrix (this matrix takes the source points to the target points)
	m_outputMatrix->DeepCopy(icp->GetMatrix());

	// Transform the source points by the ICP solution
	vtkSmartPointer<vtkTransformPolyDataFilter> icpTransformFilter =
		vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	icpTransformFilter->SetInputData(m_sourceData);
	icpTransformFilter->SetTransform(icp);
	icpTransformFilter->Update();

	// compute icp error
	vtkSmartPointer<vtkCleanPolyData> clean1 =
		vtkSmartPointer<vtkCleanPolyData>::New();
	clean1->SetInputData(icpTransformFilter->GetOutput());

	vtkSmartPointer<vtkCleanPolyData> clean2 =
		vtkSmartPointer<vtkCleanPolyData>::New();
	clean2->SetInputData(m_targetData);

	vtkSmartPointer<vtkDistancePolyDataFilter> distanceFilter =
		vtkSmartPointer<vtkDistancePolyDataFilter>::New();

	distanceFilter->SetInputConnection(0, clean1->GetOutputPort());
	distanceFilter->SetInputConnection(1, clean2->GetOutputPort());
	distanceFilter->Update();
	m_outputData->DeepCopy(distanceFilter->GetOutput());

	double error = 0;
	for (int i = 0; i < m_outputData->GetPointData()->GetArray("Distance")->GetNumberOfTuples(); i++)
	{
		error = m_outputData->GetPointData()->GetArray("Distance")->GetTuple(i)[0] + error;
	}
	error = error / m_outputData->GetPointData()->GetArray("Distance")->GetNumberOfTuples();
	m_outputError = error;
}

int ICPRegistrationFilter::Update()
{
	emit Process(50);
	this->Compute();

	emit Process(100);
	emit Finished();
	return 0;
}

