#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#include "../cl_wrapper.hpp"
#include "../../timer.hpp"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE 1024
#endif

#define binary_op(x, y) (x + y)

const char* kernelSource = STRINGIFY(
  __kernel void vector_add(__global const float *A, __global const float *B, __global float *C) {
    int id = get_global_id(0);
    C[id] = A[id] + B[id];
  }
);

float A[ARRAY_SIZE], B[ARRAY_SIZE], C[ARRAY_SIZE];
void fill_array() {
  for (int i = 0; i < ARRAY_SIZE; i++) {
    A[i] = static_cast<float>(rand() % 1000000) / 1000;
    B[i] = static_cast<float>(rand() % 1000000) / 1000;
  }
}

void gpu_test() {
    auto device = xlib::cl::get_platform().get_gpu_device();

    auto context = xlib::cl::context(device);

    auto queue = xlib::cl::command_queue(context, device);

    auto buffer_a = xlib::cl::buffer<float>(context, ARRAY_SIZE);
    auto buffer_b = xlib::cl::buffer<float>(context, ARRAY_SIZE);
    auto buffer_c = xlib::cl::buffer<float>(context, ARRAY_SIZE, CL_MEM_WRITE_ONLY);

    cl_program program;
    cl_kernel kernel;

    size_t globalSize = ARRAY_SIZE;
    size_t localSize = 64;
    cl_int err;

    buffer_a.write(queue, A);
    buffer_b.write(queue, B);

    // Компиляция программы
    program = clCreateProgramWithSource(context._context, 1, &kernelSource, NULL, &err);
    clBuildProgram(program, 1, &device.id, NULL, NULL, NULL);

    // Создание ядра
    kernel = clCreateKernel(program, "vector_add", &err);

    // Установка аргументов
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferA);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferB);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufferC);

    // Запуск ядра
    clEnqueueNDRangeKernel(queue._queue, kernel, 1, NULL, &globalSize, &localSize, 0, NULL, NULL);

    // Чтение результатов
    clEnqueueReadBuffer(queue._queue, bufferC, CL_TRUE, 0, sizeof(float) * ARRAY_SIZE, C, 0, NULL, NULL);

    // Проверка результатов
    for (int i = 0; i < 10; i++) {
        printf("C[%d] = %f\n", i, C[i]);
    }

    // Освобождение ресурсов
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
}

void cpu_test() {
  for (int i = 0; i < ARRAY_SIZE; ++i) {
    C[i] = binary_op(A[i], B[i]);
  }

  for (int i = 0; i < 10; i++) {
        printf("C[%d] = %f\n", i, C[i]);
  }
}

int main() {
  xlib::timer t;

  fill_array();

  t.restart();
  cpu_test();
  std::cout << "cpu_test(): " << t.elapsedMilliseconds() << " ms.\n";

  t.restart();
  gpu_test();
  std::cout << "gpu_test(): " << t.elapsedMilliseconds() << " ms.\n";

  return 0;
}
