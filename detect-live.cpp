// Chilitags
#include "chilitags.hpp"

// OpenCV
#include <opencv2/core/core.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/highgui/highgui.hpp>

// FIFO Pipe
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <time.h>

int main(int argc, char* argv[])
{
	// Simple parsing of the parameters related to the image acquisition
	int xRes = 640;
	int yRes = 480;
	int cameraIndex = 0;
	if (argc > 2)
	{
		xRes = std::atoi(argv[1]);
		yRes = std::atoi(argv[2]);
	}
	if (argc > 3)
	{
		cameraIndex = std::atoi(argv[3]);
	}

	// Create FIFO pipe
	int fd;
    // char* xbee_pipe = (char*)"/tmp/xbee_pipe";
    // mkfifo(xbee_pipe, 0666);

	// Create plot window to graph centers
	char tracking_window[] = "Plot";
 	cv::Mat tracking_img = cv::Mat::ones(yRes, xRes, CV_8UC3);
	tracking_img = cv::Scalar(255, 255, 255);

	// Get camera feed
	cv::VideoCapture capture(cameraIndex);
	if (!capture.isOpened())
	{
		std::cerr << "Unable to initialise video capture." << std::endl;
		return 1;
	}

	// Set camera feed properties
	capture.set(CV_CAP_PROP_FRAME_WIDTH, xRes);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, yRes);
	// capture.set(CV_CAP_PROP_EXPOSURE, 100);
	// capture.set(CV_CAP_PROP_GAIN, 50);
	// capture.set(CV_CAP_PROP_BRIGHTNESS, 150);
	// capture.set(CV_CAP_PROP_CONTRAST, 15);
	// capture.set(CV_CAP_PROP_SATURATION, 200);
	// capture.set(CV_CAP_PROP_WHITE_BALANCE_U, 30);
	// capture.set(CV_CAP_PROP_WHITE_BALANCE_V, 30);

	cv::Mat inputImage;

	chilitags::Chilitags chilitags;

	// chilitags.setFilter(0, 0.0f);

	cv::namedWindow("DisplayChilitags");

	// Main loop, exiting when 'q is pressed'
	for (int framecount = 0; 'q' != (char) cv::waitKey(1); framecount++)
	{
		capture.read(inputImage);

		chilitags::TagCornerMap tags = chilitags.find(inputImage);

		const static cv::Scalar COLOR(255, 0, 255);

		static const int SHIFT = 16;
		static const float PRECISION = 1<<SHIFT;

		cv::Mat outputImage = inputImage.clone();

		// Character buffers for displaying center of tag on page, and sending data over FIFO pipe
		char info[256];
		char toSend[256];

		// Plot target point onto plot
		cv::Point2f targetPoint;
		targetPoint.x = 100;
		targetPoint.y = 100;
		cv::circle(tracking_img, targetPoint, 1, CV_RGB(255, 0, 0), 5);

		for (const std::pair<int, chilitags::Quad> & tag : tags)
		{
			// Get ID and corner points of chilitag
			int id = tag.first;
			const cv::Mat_<cv::Point2f> corners(tag.second);

			// Draw outline around detected chilitag
			for (size_t i = 0; i < 4; ++i)
			{
				cv::line(outputImage, PRECISION*corners(i), PRECISION*corners((i+1)%4), COLOR, 1, CV_AA, SHIFT);
			}

			// Calculate center based on two corner points
			cv::Point2f center = 0.5f*(corners(0) + corners(2));

			// Calculate angle of chilitag based on two corner points
			double robotAngle = (-1 * double(atan2((corners(0).y-corners(1).y),(corners(0).x-corners(1).x))*180/CV_PI)) - 90;
			if (robotAngle < 0)
			{
				robotAngle = robotAngle + 360;
			}

			// Write center of tag onto outputImage
			sprintf(info, "x:%2.2f y:%2.2f theta:%2.1f", center.x, center.y, robotAngle);
			cv::putText(outputImage, info, center, cv::FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(0,255,0), 2.0);

			// Create string to send over to python script
			sprintf(toSend, "%d %2.1f %2.1f %2.1f\n", id, center.x, center.y, robotAngle);

			// Write to FIFO pipe
	        // fd = open(xbee_pipe, O_WRONLY);
	        // write(fd, toSend, strlen(toSend));
	        // close(fd);

	        // Plot visited points onto tacking windows
			cv::circle(tracking_img, center, 1, CV_RGB(0, 0, 0), 5);
			cv::imshow(tracking_window, tracking_img);

		}
		cv::imshow("DisplayChilitags", outputImage);
	}

	cv::destroyWindow("DisplayChilitags");
	capture.release();

	// unlink(xbee_pipe);

	return 0;
}