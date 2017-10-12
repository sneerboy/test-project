#include <openpose/filestream/fileStream.hpp>
#include <openpose/filestream/keypointSaver.hpp>
#include <iostream>

namespace op
{
    KeypointSaver::KeypointSaver(const std::string& directoryPath, const DataFormat format) :
        FileSaver{directoryPath},
        mFormat{format}
    {
    }

    void KeypointSaver::saveKeypoints(const std::vector<Array<float>>& keypointVector, const std::string& fileName, const std::string& keypointName) const
    {
        try
        {
            if (!keypointVector.empty())
            {
                // File path (no extension)
                const auto fileNameNoExtension = getNextFileName(fileName) + "_" + keypointName;

                // Get vector of people poses
                std::vector<cv::Mat> cvMatPoses(keypointVector.size());
                for (auto i = 0; i < keypointVector.size(); i++)
                    cvMatPoses[i] = keypointVector[i].getConstCvMat();

				std::cout << std::endl << "sizeof Vec : " << keypointVector.size() << std::endl;
				//std::cout << "sizeof Vec : " << keypointVector.size() << std::endl;
				//std::cout << cvMatPoses[0].size() << " " << cvMatPoses[0].elemSize();
				 
			/*	cv::Mat tmp;
				tmp.push_back(cvMatPoses[0]);
				std::cout << tmp << std::endl;
*/


//º¤ÅÍ °ª ´Ã¸®±â
				using namespace cv;
				using namespace std;


				

				std::cout << "peopleNum=" << cvMatPoses[0].size.p[1] * cvMatPoses[0].dims*cvMatPoses[0].size.p[0] / 54 << "\n";
				std::cout << "parameterNum=" << cvMatPoses[0].size.p[1] << "\n";
				std::cout << "elementNum(x,y,score)=" << cvMatPoses[0].dims << "\n";
				
				//data Ãâ·Â
				/*for (int i = 0; i < 54; i++) {
					std::cout << i << "th element" << cvMatPoses[0].at<float>(i) << "\n";
				}*/

                // Get names inside file
                std::vector<std::string> keypointVectorNames(cvMatPoses.size());
                for (auto i = 0; i < cvMatPoses.size(); i++)
                    keypointVectorNames[i] = {keypointName + "_" + std::to_string(i)};


                saveData(cvMatPoses, keypointVectorNames, fileNameNoExtension, mFormat);
			

            }
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }
}
