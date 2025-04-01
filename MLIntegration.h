#pragma once
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model.h"
#include <queue>

struct BehavioralFeatures {
    float aimSpeedVariance;
    float shotTimeDiffs[5];
    float hitProbability;
    float mouseMovementEntropy;
};

class AIModelManager {
private:
    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter;
    std::queue<BehavioralFeatures> inferenceQueue;

public:
    bool LoadModel(const char* modelPath) {
        model = tflite::FlatBufferModel::BuildFromFile(modelPath);
        tflite::ops::builtin::BuiltinOpResolver resolver;
        tflite::InterpreterBuilder(*model, resolver)(&interpreter);
        return interpreter->AllocateTensors() == kTfLiteOk;
    }

    void AddToInferenceQueue(const BehavioralFeatures& features) {
        if (inferenceQueue.size() > 100) inferenceQueue.pop();
        inferenceQueue.push(features);
    }

    float RunInference() {
        if (inferenceQueue.empty()) return -1.0f;

        float* input = interpreter->typed_input_tensor<float>(0);
        interpreter->Invoke();
        
        float* output = interpreter->typed_output_tensor<float>(0);
        return output[0]; 
    }
};
