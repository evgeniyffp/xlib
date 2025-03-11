#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define STRINGIFY(x) #x

const char *kernelSource = STRINGIFY(
    __kernel void blur_filter(
        __global const uchar *inputImage,
        __global uchar *outputImage,
        const int width,
        const int height
    ) {
        int x = get_global_id(0);
        int y = get_global_id(1);

        int sum = 0;
        int count = 0;

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int nx = x + dx;
                int ny = y + dy;

                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                    sum += inputImage[ny * width + nx];
                    count++;
                }
            }
        }

        outputImage[y * width + x] = (uchar)(sum / count);
    }
);

#define CHECK_ERROR(err, msg) \
    if (err != CL_SUCCESS) { \
        fprintf(stderr, "Error: %s (%d)\n", msg, err); \
        exit(EXIT_FAILURE); \
    }

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <input_image> <output_image>\n", argv[0]);
        return 1;
    }

    // Шаг 1: Загрузка изображения
    int width, height, channels;
    unsigned char *inputImage = stbi_load(argv[1], &width, &height, &channels, 1); // Загрузка в оттенках серого
    if (!inputImage) {
        fprintf(stderr, "Failed to load image: %s\n", argv[1]);
        return 1;
    }

    unsigned char *outputImage = (unsigned char *)malloc(width * height);
    if (!outputImage) {
        fprintf(stderr, "Failed to allocate memory for output image\n");
        stbi_image_free(inputImage);
        return 1;
    }

    // Шаг 2: Инициализация OpenCL
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem bufferInput, bufferOutput;
    size_t globalSize[2];
    globalSize[0] = width;
    globalSize[1] = height;
    cl_int err;

    err = clGetPlatformIDs(1, &platform, NULL);
    CHECK_ERROR(err, "Failed to get platform ID");

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    CHECK_ERROR(err, "Failed to get device ID");

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    CHECK_ERROR(err, "Failed to create context");

    queue = clCreateCommandQueue(context, device, 0, &err);
    CHECK_ERROR(err, "Failed to create command queue");

    // Создаем буферы OpenCL
    bufferInput = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned char) * width * height, NULL, &err);
    CHECK_ERROR(err, "Failed to create input buffer");

    bufferOutput = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(unsigned char) * width * height, NULL, &err);
    CHECK_ERROR(err, "Failed to create output buffer");

    // Записываем данные изображения в буфер
    err = clEnqueueWriteBuffer(queue, bufferInput, CL_TRUE, 0, sizeof(unsigned char) * width * height, inputImage, 0, NULL, NULL);
    CHECK_ERROR(err, "Failed to write to input buffer");

    // Создаем и компилируем программу
    program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, &err);
    CHECK_ERROR(err, "Failed to create program");

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        char *log = (char *)malloc(logSize);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log, NULL);
        fprintf(stderr, "Build Log:\n%s\n", log);
        free(log);
        exit(EXIT_FAILURE);
    }

    kernel = clCreateKernel(program, "blur_filter", &err);
    CHECK_ERROR(err, "Failed to create kernel");

    // Устанавливаем аргументы ядра
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferInput);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferOutput);
    clSetKernelArg(kernel, 2, sizeof(int), &width);
    clSetKernelArg(kernel, 3, sizeof(int), &height);

    // Запускаем ядро
    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalSize, NULL, 0, NULL, NULL);
    CHECK_ERROR(err, "Failed to enqueue kernel");

    // Читаем результат
    err = clEnqueueReadBuffer(queue, bufferOutput, CL_TRUE, 0, sizeof(unsigned char) * width * height, outputImage, 0, NULL, NULL);
    CHECK_ERROR(err, "Failed to read output buffer");

    // Сохраняем результат
    if (!stbi_write_png(argv[2], width, height, 1, outputImage, width)) {
        fprintf(stderr, "Failed to write output image: %s\n", argv[2]);
    }

    // Освобождаем ресурсы
    clReleaseMemObject(bufferInput);
    clReleaseMemObject(bufferOutput);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    stbi_image_free(inputImage);
    free(outputImage);

    printf("Image processing complete. Output saved to %s\n", argv[2]);
    return 0;
}

