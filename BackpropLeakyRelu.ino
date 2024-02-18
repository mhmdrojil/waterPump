#include <Adafruit_TFLite.h>
// Definisikan hyperparameter untuk pelatihan
#define learning_rate 0.1
#define epochs 1000

// Definisikan Leaky ReLU threshold
#define alpha 0.1

// Data latihan dan target
float train_data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
float train_target[] = {2.0, 4.0, 6.0, 8.0, 10.0};

// Inisialisasi bobot dan bias secara acak
float weights[] = {0.5};
float bias = 0.5;

// Definisikan fungsi aktivasi Leaky ReLU
float leaky_relu(float x) {
  return x >= 0 ? x : alpha * x;
  }
}

// Definisikan turunan fungsi aktivasi Leaky ReLU
float leaky_relu_derivative(float x) {
  return x >= 0 ? 1 : alpha;
  }
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  // Pelatihan jaringan dengan backpropagation
  for (int epoch = 0; epoch < epochs; epoch++) {
    float total_error = 0.0;

    for (int i = 0; i < 5; i++) {
      // Forward pass
      float input_data = train_data[i];
      float target = train_target[i];
      
      float output = input_data * weights[0] + bias;
      output = leaky_relu(output);
      
      // Menghitung error
      float error = target - output;
      total_error += error * error;
      
      // Backward pass
      // Menghitung gradien berdasarkan error
      float delta = error * leaky_relu_derivative(output);
      
      // Memperbarui bobot dan bias
      weights[0] += learning_rate * delta * input_data;
      bias += learning_rate * delta;
    }
    
    total_error /= 5; // Menghitung rata-rata error
    Serial.print("Epoch: ");
    Serial.print(epoch);
    Serial.print(", Error: ");
    Serial.println(total_error);
  }

  // Tes model setelah pelatihan
  Serial.println("Testing the trained model:");
  for (int i = 0; i < 5; i++) {
    float input_data = train_data[i];
    float target = train_target[i];
    
    float output = input_data * weights[0] + bias;
    output = leaky_relu(output);
    
    Serial.print("Input: ");
    Serial.print(input_data);
    Serial.print(", Target: ");
    Serial.print(target);
    Serial.print(", Predicted: ");
    Serial.println(output);
  }

  // Hentikan program agar tidak terus berjalan
  while (true) {
    ;
  }
}
