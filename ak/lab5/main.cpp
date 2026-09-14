#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;
int g_slider_position = 100;
int main()
{
  VideoCapture capture(0);
  if (!capture.isOpened()) {
    cout << "Камера не найдена" << endl;
    return -1;
  }
  int width = (int)capture.get(CAP_PROP_FRAME_WIDTH);
  int height = (int)capture.get(CAP_PROP_FRAME_HEIGHT);
  Size original_size(width, height);
  VideoWriter writer("output.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), 30.0, original_size);
  namedWindow("camera", WINDOW_AUTOSIZE);
  createTrackbar("Zoom", "camera", &g_slider_position, 1000);
  Mat frame, resized_frame;
  while (true)
  {
    capture >> frame;
    if (frame.empty()) break;
    writer.write(frame);
double zoom_factor = g_slider_position / 100.0;
if (zoom_factor < 1.0) zoom_factor = 1.0;

int new_w = (int)(width / zoom_factor);
int new_h = (int)(height / zoom_factor);

int x = (width - new_w) / 2;
int y = (height - new_h) / 2;

Rect rect(x, y, new_w, new_h);

Mat cropped_frame = frame(rect);
resize(cropped_frame, resized_frame, original_size, 0, 0, INTER_LINEAR);


Mat styled_frame;

stylization(resized_frame, styled_frame, 60, 0.45f);

imshow("camera", styled_frame);

char c = (char)waitKey(33);
if (c == 27) break;  }
  return 0;
}
