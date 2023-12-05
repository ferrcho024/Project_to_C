# Portando al ESP32

Para portar a microcontroladores, siga el siguiente link: https://www.tensorflow.org/lite/microcontrollers?hl=es-419

Luego nos vamos al siguiente enlace: https://www.tensorflow.org/lite/microcontrollers/get_started_low_level?hl=es-419

Esta parte se asocia a la prueba unitaria, la cual es bueno hacer antes de hacer el montaje en el microcontrolador para verificar que todo esta en orden.

1. Incluya los encabezados de la libreria TensorFlow Lite for Microcontrollers

    ```c
    #include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
    #include "tensorflow/lite/micro/micro_error_reporter.h"
    #include "tensorflow/lite/micro/micro_interpreter.h"
    #include "tensorflow/lite/schema/schema_generated.h"
    #include "tensorflow/lite/version.h"
    ```

2. Incluya el encabezado del modelo:
       
    ```c
    #include "tensorflow/lite/micro/examples/hello_world/model.h"
    ```

3. Incluya el encabezado del marco de prueba de unidad:
   
   ```c
   #include "tensorflow/lite/micro/testing/micro_test.h"
   ```

   Para definir la prueba use las macros:
   
    ```c
    TF_LITE_MICRO_TESTS_BEGIN
    
    TF_LITE_MICRO_TEST(LoadModelAndPerformInference) {
      . // add code here
      .
    }
    
    TF_LITE_MICRO_TESTS_END
    ```

4. Configurar el registro:

    ```c
    tflite::MicroErrorReporter micro_error_reporter;
    tflite::ErrorReporter* error_reporter = &micro_error_reporter;
    ```

5. Carge el modelo:
   
    ```c
    const tflite::Model* model = ::tflite::GetModel(g_model);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
      TF_LITE_REPORT_ERROR(error_reporter,
          "Model provided is schema version %d not equal "
          "to supported version %d.\n",
          model->version(), TFLITE_SCHEMA_VERSION);
    }
    ```

6. Instanciar operaciones de resolución:
   
    ```c
    using HelloWorldOpResolver = tflite::MicroMutableOpResolver<1>;
    
    TfLiteStatus RegisterOps(HelloWorldOpResolver& op_resolver) {
      TF_LITE_ENSURE_STATUS(op_resolver.AddFullyConnected());
      return kTfLiteOk;
    
    ```

    ```c
    HelloWorldOpResolver op_resolver;
    TF_LITE_ENSURE_STATUS(RegisterOps(op_resolver));
    ```

7. Asignar memoria 


    ```c
    const int tensor_arena_size = 2 * 1024;
    uint8_t tensor_arena[tensor_arena_size];
    ```
    
    El tamaño requerido dependerá del modelo que esté utilizando y es posible que deba determinarse mediante experimentación.

8. Instanciar intérprete

    ```c
    tflite::MicroInterpreter interpreter(model, resolver, tensor_arena,
                                         tensor_arena_size, error_reporter);
    ```

9. Asignar tensores

    ```c
    interpreter.AllocateTensors();
    ```

10. Validar la forma de entrada

    ```c
    // Obtain a pointer to the model's input tensor
    TfLiteTensor* input = interpreter.input(0);
    ```

    Luego este tensor para confirmar que su forma y tipo son los esperado:

    ```c
    // Make sure the input has the properties we expect
    TF_LITE_MICRO_EXPECT_NE(nullptr, input);
    // The property "dims" tells us the tensor's shape. It has one element for
    // each dimension. Our input is a 2D tensor containing 1 element, so "dims"
    // should have size 2.
    TF_LITE_MICRO_EXPECT_EQ(2, input->dims->size);
    // The value of each element gives the length of the corresponding tensor.
    // We should expect two single element tensors (one is contained within the
    // other).
    TF_LITE_MICRO_EXPECT_EQ(1, input->dims->data[0]);
    TF_LITE_MICRO_EXPECT_EQ(1, input->dims->data[1]);
    // The input is a 32 bit floating point value
    TF_LITE_MICRO_EXPECT_EQ(kTfLiteFloat32, input->type);
    ```

11. Proporcione un valor de entrada

    ```c
    input->data.f[0] = 0.;
    ```

12. Ejecutar el modelo:

    ```c
    TfLiteStatus invoke_status = interpreter.Invoke();
    if (invoke_status != kTfLiteOk) {
      TF_LITE_REPORT_ERROR(error_reporter, "Invoke failed\n");
    }
    ```
    
    Ademas:
    
    ```c
    TF_LITE_MICRO_EXPECT_EQ(kTfLiteOk, invoke_status);
    ```

13. Obtenga la salida

    ```c
    TfLiteTensor* output = interpreter.output(0);
    TF_LITE_MICRO_EXPECT_EQ(2, output->dims->size);
    TF_LITE_MICRO_EXPECT_EQ(1, input->dims->data[0]);
    TF_LITE_MICRO_EXPECT_EQ(1, input->dims->data[1]);
    TF_LITE_MICRO_EXPECT_EQ(kTfLiteFloat32, output->type);
    ```

    Podemos leer el valor directamente del tensor de salida y confirmar que es lo que esperamos:
    
    ```c
    // Obtain the output value from the tensor
    float value = output->data.f[0];
    // Check that the output value is within 0.05 of the expected value
    TF_LITE_MICRO_EXPECT_NEAR(0., value, 0.05);
    ```

14. Vuelva a ejecutar la inderencia

    ```c
    input->data.f[0] = 1.;
    interpreter.Invoke();
    value = output->data.f[0];
    TF_LITE_MICRO_EXPECT_NEAR(0.841, value, 0.05);
    
    input->data.f[0] = 3.;
    interpreter.Invoke();
    value = output->data.f[0];
    TF_LITE_MICRO_EXPECT_NEAR(0.141, value, 0.05);
    
    input->data.f[0] = 5.;
    interpreter.Invoke();
    value = output->data.f[0];
    TF_LITE_MICRO_EXPECT_NEAR(-0.959, value, 0.05);
    ```

## Procediendo a descargar las cosas en el micro

Codigos:
* Codigo del libro: https://github.com/tensorflow/tensorflow/tree/be4f6874533d78f662d9777b66abe3cdde98f901/tensorflow/lite/experimental/micro/examples/hello_world/arduino
* Codigo actual: 

La explicación se encuentra tambien en: https://www.digikey.com/en/maker/projects/intro-to-tinyml-part-1-training-a-model-for-arduino-in-tensorflow/8f1fc8c0b83d417ab521c48864d2a8ec, mejor dicho creo que para empezar la derecha es seguir este tutorial.

Para ver la forma de la red, puede cargar el modelo comprimido en: https://netron.app/



