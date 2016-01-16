#include "highgui.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "ovr.h"
#include "gcube.h"
#include "gpu_util.h"


// can be 30-45ish
#define DCAMFPS 30

int main() {
  // open both left and right cameras
  cv::VideoCapture left(2);
  cv::VideoCapture right(1);
  assert(left.isOpened() && right.isOpened());

  // set properties
  left.set(CV_CAP_PROP_FRAME_WIDTH, 640); 
  left.set(CV_CAP_PROP_FRAME_HEIGHT, 480); 
  left.set(CV_CAP_PROP_FPS, DCAMFPS); 
  right.set(CV_CAP_PROP_FRAME_WIDTH, 640); 
  right.set(CV_CAP_PROP_FRAME_HEIGHT, 480); 
  right.set(CV_CAP_PROP_FPS, DCAMFPS);

  // (optional) try to identify the different cameras
/*  double gain1 = left.get(CV_CAP_PROP_GAIN);
  double gain2 = right.get(CV_CAP_PROP_GAIN);
  bool swapped = false;
  if (gain1 > 0.6 && gain2 < 0.6) {
    swapped = true;
  }
*/
  // grab and display the frame
  cv::namedWindow("hud");
  cv::Mat frames[2];
  double offset = 0.15;
  gcube limg, rimg, combined;
  cv::Mat out;

  for (;;) {
    left.grab();
    right.grab();
    left.retrieve(frames[0]);
    right.retrieve(frames[1]);
    if (!frames[0].data || !frames[1].data) {
      printf("No data...\n");
      continue;
    }
    // statically mapped numbers - assumption is that the frame is going to be 480x640 big
    limg.create(frames[0], 128, 511, 0, 480);
    rimg.create(frames[1], 128, 511, 0, 480);
    combined = ovr_image(limg, rimg, offset); // waste copy
    out = combined.cv_img();
    cv::imshow("hud", out);
    if (cv::waitKey(DCAMFPS * 2 / 3) >= 0) {
      break;
    }
  }
  return 0;
}
