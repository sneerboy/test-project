#include <chrono>
#include <cstdio> // std::snprintf
#include <limits> // std::numeric_limits
#include <openpose/utilities/fastMath.hpp>
#include <openpose/utilities/openCv.hpp>
#include <openpose/gui/guiInfoAdder.hpp>

#include <openpose/core/headers.hpp>
#include <openpose/filestream/headers.hpp>
#include <openpose/gui/headers.hpp>
#include <openpose/pose/headers.hpp>
#include <openpose/utilities/headers.hpp>

#include <iostream>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\core\core.hpp>
#include <opencv\cv.hpp>


extern int joon1 = 9;

namespace op
{
    void updateFps(unsigned long long& lastId, double& fps, unsigned int& fpsCounter,
                   std::queue<std::chrono::high_resolution_clock::time_point>& fpsQueue,
                   const unsigned long long id, const int numberGpus)
    {
        try
        {
			
            // If only 1 GPU -> update fps every frame.
            // If > 1 GPU:
                // We updated fps every (3*numberGpus) frames. This is due to the variability introduced by
                // using > 1 GPU at the same time.
                // However, we update every frame during the first few frames to have an initial estimator.
            // In any of the previous cases, the fps value is estimated during the last several frames.
            // In this way, a sudden fps drop will be quickly visually identified.
            if (lastId != id)
            {
                lastId = id;
                fpsQueue.emplace(std::chrono::high_resolution_clock::now());
                bool updatePrintedFps = true;
                if (fpsQueue.size() > 5)
                {
                    const auto factor = (numberGpus > 1 ? 25u : 15u);
                    updatePrintedFps = (fpsCounter % factor == 0);
                    // updatePrintedFps = (numberGpus == 1 ? true : fpsCounter % (3*numberGpus) == 0);
                    fpsCounter++;
                    if (fpsQueue.size() > factor)
                        fpsQueue.pop();
                }
                if (updatePrintedFps)
                {
                    const auto timeSec = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                        fpsQueue.back()-fpsQueue.front()
                    ).count() * 1e-9;
                    fps = (fpsQueue.size()-1) / (timeSec != 0. ? timeSec : 1.);
                }
            }
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }

    GuiInfoAdder::GuiInfoAdder(const Point<int>& outputSize, const int numberGpus, const bool guiEnabled) :
        mOutputSize{outputSize},
        mBorderMargin{intRound(fastMax(mOutputSize.x, mOutputSize.y) * 0.025)},
        mNumberGpus{numberGpus},
        mGuiEnabled{guiEnabled},
        mFpsCounter{0u},
        mLastElementRenderedCounter{std::numeric_limits<int>::max()},
        mLastId{std::numeric_limits<unsigned long long>::max()}
    {
    }

    void GuiInfoAdder::addInfo(cv::Mat& cvOutputData, const Array<float>& poseKeypoints, const unsigned long long id,
                               const std::string& elementRenderedName)
    {
		try
		{
			// Security checks
		

			if (cvOutputData.empty())
				error("Wrong input element (empty cvOutputData).", __LINE__, __FUNCTION__, __FILE__);
			// Update fps
			updateFps(mLastId, mFps, mFpsCounter, mFpsQueue, id, mNumberGpus);
			// Used colors
			const cv::Scalar white{ 255, 255, 255 };
			// Fps or s/gpu
			char charArrayAux[15];
			std::snprintf(charArrayAux, 15, "%4.1f fps test", mFps);
			// Recording inverse: sec/gpu
			// std::snprintf(charArrayAux, 15, "%4.2f s/gpu", (mFps != 0. ? mNumberGpus/mFps : 0.));
			putTextOnCvMat(cvOutputData, charArrayAux, { intRound(mOutputSize.x - mBorderMargin), mBorderMargin },
				white, true);
			// Part to show
			// Allowing some buffer when changing the part to show (if >= 2 GPUs)
			// I.e. one GPU might return a previous part after the other GPU returns the new desired part, it looks
			// like a mini-bug on screen
			// Difference between Titan X (~110 ms) & 1050 Ti (~290ms)
			if (mNumberGpus == 1 || (elementRenderedName != mLastElementRenderedName
				&& mLastElementRenderedCounter > 4))
			{
				mLastElementRenderedName = elementRenderedName;
				mLastElementRenderedCounter = 0;
			}
			mLastElementRenderedCounter = fastMin(mLastElementRenderedCounter, std::numeric_limits<int>::max() - 5);
			mLastElementRenderedCounter++;
			// OpenPose name as well as help or part to show
			putTextOnCvMat(cvOutputData, "OpenPose - " +
				(!mLastElementRenderedName.empty() ?
					mLastElementRenderedName : (mGuiEnabled ? "'h' for help" : "")),
					{ mBorderMargin, mBorderMargin }, white, false);
			// Frame number
			putTextOnCvMat(cvOutputData, "Frame: " + std::to_string(id),
			{ mBorderMargin, (int)(mOutputSize.y - mBorderMargin) }, white, false);
			// Number people
			putTextOnCvMat(cvOutputData, "People: " + std::to_string(poseKeypoints.getSize(0)),
			{ (int)(mOutputSize.x - mBorderMargin), (int)(mOutputSize.y - mBorderMargin) }, white, true);

			cv::Mat cvMatPoses;
			cvMatPoses = poseKeypoints.getConstCvMat();

			 

			std::vector<cv::Mat> test_load;
			//std::vector<std::string> keypointVectorNames(cvMatPoses.size());


			//op::loadData()
			//test_load = op::loadData(test_load, "aa_pose", op::stringToDataFormat("yml"));
			//std::cout << test_load;



			char body_point[18][12] = { "Head", "Neck",
				"RShoulder","RElbow","RWrist",
				"LShoulder","LElbow","LWrist",
				"RHip","RKnee","RAnkle",
				"LHip","LKnee","LAnkle",
				"Chest","Background" };

			//for (int i = 0; i < 10; i+=3) {
			//	//std::cout << i << "th element" << cvMatPoses.at<float>(i) << "\n";
			//	char output_txt[15];
			//	std::snprintf(output_txt, 15, "%s", body_point[i/3]);
			//	if (cvMatPoses.at<float>(i)!=0 && cvMatPoses.at<float>(i+1)!=0) {
			//		putTextOnCvMat(cvOutputData, output_txt,
			//		{ (int)(cvMatPoses.at<float>(i)), (int)(cvMatPoses.at<float>(i+1)) }, white, true);
			//	}
			//}
			int x1 = (int)(cvMatPoses.at<float>(3)), y1 = (int)(cvMatPoses.at<float>(4));
			int x2 = (int)(cvMatPoses.at<float>(6)), y2 = (int)(cvMatPoses.at<float>(7));
			int x3 = (int)(cvMatPoses.at<float>(9)), y3 = (int)(cvMatPoses.at<float>(10));
			int x4 = (int)(cvMatPoses.at<float>(12)), y4 = (int)(cvMatPoses.at<float>(13));
			int x5 = (int)(cvMatPoses.at<float>(14)), y5 = (int)(cvMatPoses.at<float>(15));
			int x6 = (int)(cvMatPoses.at<float>(17)), y6 = (int)(cvMatPoses.at<float>(18));
			int x7 = (int)(cvMatPoses.at<float>(20)), y7 = (int)(cvMatPoses.at<float>(21));

			float vec21[4] = { x1 - x2, y1 - y2, 0 };
			float vec32[4] = { x2 - x3, y3 - y2, 0};
			float vec34[4] = { x4 - x3, y3 - y4, 0}; 
			float vec23[4] = { (-1)*vec32[0] , (-1)*vec32[1] , 0 };

			//std::cout << vec32[0] << " " << vec32[1] << std::endl;
			if (vec32[0] != 0 && vec34[0] != 0 && vec32[1] != 0 && vec34[1] != 0) {
				vec32[2] = sqrt(pow(vec32[0], 2) + pow(vec32[1], 2));
				vec34[2] = sqrt(pow(vec34[0], 2) + pow(vec34[1], 2));
				vec23[2] = vec32[2];

				float dot_product = (vec32[0] * vec34[0] + vec32[1] * vec34[1]) / (vec32[2] * vec34[2]);
				float cross_product = (vec32[0] * vec34[1]) - (vec32[1] * vec34[0]);
				float tmp_angle;
				
				tmp_angle = acos(dot_product) * 180.0 / 3.14159265;
				if (cross_product < 0) tmp_angle *= -1;
				joon1 = tmp_angle;
				char output_txt[30];
				std::snprintf(output_txt, 15, "angle: %.2f", tmp_angle);
				
				putTextOnCvMat(cvOutputData, output_txt,
				{ (int)(cvMatPoses.at<float>(9)), (int)(cvMatPoses.at<float>(10)) }, white, true);
			}
			

			if (vec23[0] != 0 && vec21[0] != 0 && vec23[1] != 0 && vec21[1] != 0) {
				vec32[2] = sqrt(pow(vec32[0], 2) + pow(vec32[1], 2));
				vec34[2] = sqrt(pow(vec34[0], 2) + pow(vec34[1], 2));
				vec23[2] = vec32[2];

				float dot_product = (vec32[0] * vec34[0] + vec32[1] * vec34[1]) / (vec32[2] * vec34[2]);
				float cross_product = (vec32[0] * vec34[1]) - (vec32[1] * vec34[0]);
				float tmp_angle;

				tmp_angle = acos(dot_product) * 180.0 / 3.14159265;
				if (cross_product < 0) tmp_angle *= -1;
				joon1 = tmp_angle;
				char output_txt[30];
				std::snprintf(output_txt, 15, "angle: %.2f", tmp_angle);

				putTextOnCvMat(cvOutputData, output_txt,
				{ (int)(cvMatPoses.at<float>(9)), (int)(cvMatPoses.at<float>(10)) }, white, true);
			}

	
        }
        catch (const std::exception& e)
        {
            error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }
}
