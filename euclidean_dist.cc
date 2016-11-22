#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"
#include "tensorflow/core/framework/op_kernel.h"
#include <cmath>

using namespace tensorflow;

REGISTER_OP("EuclideanDist")
    .Input("data: T")
    .Input("clusters: T")
    .Output("distances: T")
    .Attr("T: {half, float, double, int32, int64, complex64, complex128}")
    .SetShapeFn([](::tensorflow::shape_inference::InferenceContext* c) {

      ::tensorflow::shape_inference::ShapeHandle a;
      TF_RETURN_IF_ERROR(c->WithRank(c->input(0), 2, &a));

      ::tensorflow::shape_inference::ShapeHandle b;
      TF_RETURN_IF_ERROR(c->WithRank(c->input(1), 2, &b));

      ::tensorflow::shape_inference::DimensionHandle output_rows = c->Dim(a, 0);
      ::tensorflow::shape_inference::DimensionHandle output_cols = c->Dim(b, 1);

      // Validate that the inner shapes are compatible.
      ::tensorflow::shape_inference::DimensionHandle inner_a = c->Dim(a, 1);
      ::tensorflow::shape_inference::DimensionHandle inner_b = c->Dim(b, 0);
      ::tensorflow::shape_inference::DimensionHandle merged;
      TF_RETURN_IF_ERROR(c->Merge(inner_a, inner_b, &merged));

      c->set_output(0, c->Matrix(output_rows, output_cols));
      return Status::OK();
    })
    .Doc(R"doc(
Computes the euclidean distance from every vector in "data" to every
vector in "clusters". The inputs must be two-dimensional matrices and
the inner dimension of "data" must match the inner dimension of "clusters".
)doc");


class EuclideanDistOp : public OpKernel {
 public:
  EuclideanDistOp(OpKernelConstruction* context) : OpKernel(context) {}

  void Compute(OpKernelContext* context) override {
    // Grab the input tensor
    const Tensor& input_tensor1 = context->input(0);
    const Tensor& input_tensor2 = context->input(1);

    const int r1=input_tensor1.shape().dim_size(0); //num of data
    const int c1=input_tensor1.shape().dim_size(1); //dimensions

    const int r2=input_tensor2.shape().dim_size(0); //dimensions
    const int c2=input_tensor2.shape().dim_size(1); //clusters


    auto input1 = input_tensor1.shaped<double, 2>({input_tensor1.shape().dim_size(0),input_tensor1.shape().dim_size(1)});
    auto input2 = input_tensor2.shaped<double, 2>({input_tensor2.shape().dim_size(0),input_tensor2.shape().dim_size(1)});


    //printf("%d",input1(0,0));
    //printf("%d",input2(0,0));

    // Create an output tensor
    Tensor* output_tensor = NULL;

    TensorShape out_shape=  input_tensor1.shape();

    out_shape.set_dim(1, c2);


    OP_REQUIRES_OK(context, context->allocate_output(0, out_shape,
                                                     &output_tensor));

    auto output = output_tensor->shaped<double, 2>({r1,c2});

    // Set all but the first element of the output tensor to 0.
    //const int r1 = input1.size();
    //const int c1 = input1.size();

    /* Standard Matrix Multiplication */
    /*	
    for (int i = 0; i < r1; i++) {
	for (int j=0; j< c2; j++){
		for (int k=0; k<c2; k++){			
			output(i,j) += input1(i,k)*input2(k,j);
		}
	}
    }
    */

    
    for(int n=0; n<r1; n++){
	for (int k=0; k<c2; k++){
		for (int d=0; d<c1; d++){
			output(n,k)+=pow((input1(n,d)-input2(k,d)),2);
		}
	}
    }

    for(int n=0; n<r1; n++){
	for(int k=0; k<c2; k++){
		output(n,k)=sqrt(output(n,k));
	}
    }

    
  }
};


REGISTER_KERNEL_BUILDER(Name("EuclideanDist").Device(DEVICE_CPU), EuclideanDistOp);