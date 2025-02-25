#include <vector>
#include "tensorflow/core/public/session.h"
#include "opencv2/opencv.hpp"
using namespace std;
using namespace tensorflow;
using namespace cv;
using tensorflow::Tensor;
using tensorflow::Status;
using tensorflow::string;

#include "tensorflow/cc/saved_model/loader.h"
#include "tensorflow/cc/saved_model/signature_constants.h"
#include "tensorflow/cc/saved_model/tag_constants.h"

void convertMat2Tensor(Mat img, Tensor* output_tensor, int height, int width) {
  resize(img, img, cv::Size(height, width));
  float *p = output_tensor->flat<float>().data();
  Mat output_Mat(height, width, CV_32FC1, p);
  img.convertTo(output_Mat, CV_32FC1);
}

int main(int argc, char** argv )
{
  //Read image
  string image_path = "../digit_one.jpg";
  int height = 28;
  int width = 28;
  Mat img = imread(image_path);
  if (img.empty()) {
      cout << "ERROR: Opening image failed successfully !" << endl;
      return -1;
  }
  
  //Create input tensor and output tensor
  Tensor input_tensor(DT_FLOAT, TensorShape({1, height, width, 1}));
  convertMat2Tensor(img, &input_tensor, height, width);
  vector<Tensor> output_tensors;
  string input_node_name = "inputs";
  string output_node_name = "softmax";

  //Load model
  string model_path = "../model.pb";
  GraphDef graphdef;
  Status status_model_load = ReadBinaryProto(Env::Default(), model_path, &graphdef);
  if (!status_model_load.ok()) {
      cout << "ERROR: Loading model to graph failed successfully!" << endl; 
      cout << status_model_load.ToString() << endl;
      return -1;
  }
  cout << "INFO: Loading model to graph successfully !" << endl;
  
  // load saved_model as saved_model_bundle_lite
  
    //Read image
  string image_path = "~/idsia_trails_dataset/001/videos/lc/GOPR0006.MP4.frames/00000001.jpg";
  Mat img1 = imread(image_path);
  if (img1.empty()) {
      cout << "ERROR: Opening image failed successfully !" << endl;
      return -1;
  }
  
  cvtColor(img1, img1, COLOR_BGR2RGB);
  resize(img1, img1, Size(224,224));
  // skalowanie do [-1,1]
  
  Tensor input_tensor1(DT_FLOAT, TensorShape({1, 224,224, 1}));
  convertMat2Tensor(img, &input_tensor1, 224,224);
 
  
  const string export_dir = "../my_model";
  SessionOptions session_options;  
  session_options.config.mutable_gpu_options()->set_allow_growth(true);
  SavedModelBundleLite bundle;
  RunOptions run_options;

  LoadSavedModel(session_options, run_options, export_dir, {kSavedModelTagServe},
               &bundle);
  
  const auto& signature_def = bundle.GetSignatures().at("serving_default");
  
  const string input_name = signature_def.inputs().at("input_1").name();
  const string output_name =
      signature_def.outputs().at("activation").name();
  
  cout << "input_name: " << input_name << "output_name: " << output_name << endl;
  
  std::vector<Tensor> outputs;
  bundle.GetSession()->Run({{input_name, input_tensor1}}, {output_name},
                                        {}, &outputs);

  cout << "outputs size: " << outputs.size() << endl;
  
  /*
  int nclasses = outputs[0].shape().dim_size(1);
  for (int i = 0; i < nclasses; i++) {
      cout << "Class " << i << " Prob: " << outputs[0].tensor<float, 2>()(0, i) << endl;

  }*/

  //Create graph in session
  Session* session;
  
  SessionOptions sessionOptions;
  
  sessionOptions.config.mutable_gpu_options()->set_allow_growth(true);
  

  
  NewSession(sessionOptions, &session);
  Status status_create = session->Create(graphdef);
  if (!status_create.ok()) {
     cout << "ERROR: Creating graph in session failed successfully!" << endl;
     cout << status_create.ToString() << endl;
     return -1;  
  }
  cout << "INFO: Session successfully created!"<< endl;
  
  Status status_run = session->Run({{input_node_name, input_tensor}}, {output_node_name}, {}, &output_tensors);
  if (!status_run.ok()) {
      cout << "ERROR: Session run failed successfully !" << endl;
      cout << status_run.ToString() << endl;
      return -1;      
  }
  int output_dim = output_tensors[0].shape().dim_size(1);
  for (int i = 0; i < output_dim; i++) {
      cout << "Class " << i << " Prob: " << output_tensors[0].tensor<float, 2>()(0, i) << endl;

  }
  return 0;
}
