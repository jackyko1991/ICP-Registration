# include "vtkSTLReader.h"
# include "vtkSmartPointer.h"
# include "vtkPolyData.h"
# include "vtkXMLPolyDataWriter.h"

# include "ICPRegistrationFilter.h"

int main()
{
	vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
	reader->SetFileName("C:/Users/DE/Desktop/CBCT_icp/data/CBCT_pre_reg.stl");
	reader->Update();

	vtkSmartPointer<vtkPolyData> moving = vtkSmartPointer <vtkPolyData>::New();
	moving->DeepCopy(reader->GetOutput());

	reader->SetFileName("C:/Users/DE/Desktop/CBCT_icp/data/auto_registration_odontoprisis.stl");
	reader->Update();

	vtkSmartPointer<vtkPolyData> fixed = vtkSmartPointer <vtkPolyData>::New();
	fixed->DeepCopy(reader->GetOutput());

	ICPRegistrationFilter* icpFilter = new ICPRegistrationFilter;
	icpFilter->SetSourcePolyData(moving);
	icpFilter->SetTargetPolyData(fixed);
	icpFilter->Update();

	vtkSmartPointer<vtkPolyData> output = vtkSmartPointer<vtkPolyData>::New();
	icpFilter->GetOutputData(output);

	std::cout << icpFilter->GetOutputError() << std::endl;

	vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
	writer->SetFileName("C:/Users/DE/Desktop/CBCT_icp/data/CBCT_reg.vtp");
	writer->SetInputData(output);
	writer->Write();

	system("pause");


	

}