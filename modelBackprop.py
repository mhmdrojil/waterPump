import numpy as np
import tensorflow as tf

input_size = 3
hidden_size = 5
output_size = 1

# Buat model dengan menggunakan TensorFlow's Sequential API
model = tf.keras.models.Sequential([
    tf.keras.layers.Dense(hidden_size, activation=tf.nn.leaky_relu, input_shape=(input_size,)),
    tf.keras.layers.Dense(output_size, activation=tf.nn.leaky_relu)
])

loss_function = tf.keras.losses.mean_squared_error
optimizer = tf.keras.optimizers.SGD(learning_rate=0.01)

model.compile(loss=loss_function, optimizer=optimizer)

data_latih = np.array([[ 0.75, 0.60, 0.70 ],
                       [ 0.55, 0.25, 1.00 ],
                       [ 0.45, 1.00, 0.50 ],
                       [ 0.90, 1.00, 0.95 ],
                       [ 0.25, 0.45, 0.50 ],
                       [ 1.00, 0.75, 0.85 ],
                       [ 1.00, 0.00, 0.00 ],
                       [ 0.00, 1.00, 1.00 ]
                       ])

target_latih = np.array([[0],
                         [1],
                         [1],
                         [0],
                         [1],
                         [0],
                         [0],
                         [1]
                         ])

jumlah_epochs = 7000
model.fit(data_latih, target_latih, epochs=jumlah_epochs)

for layer in model.layers:
    if hasattr(layer, 'weights'):
        print(f"Layer: {layer.name}")
        weights, biases = layer.get_weights()
        print(f"Weights:\n{weights}")
        print(f"Biases:\n{biases}")