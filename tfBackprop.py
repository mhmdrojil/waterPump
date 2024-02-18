import pandas as pd
import numpy as np
import tensorflow as tf

input_size = 3
hidden_size = 5
output_size = 1

model = tf.keras.models.Sequential([
    tf.keras.layers.Dense(hidden_size, activation=tf.nn.leaky_relu, input_shape=(input_size,)),
    tf.keras.layers.Dense(output_size, activation=tf.nn.leaky_relu)
])

loss_function = tf.keras.losses.mean_squared_error
optimizer = tf.keras.optimizers.SGD(learning_rate=0.01)

model.compile(loss=loss_function, optimizer=optimizer)

data_frame = pd.read_csv('dataTrain.csv')
print(data_frame)
data_latih = data_frame.drop(columns=['target']).values
target_latih = data_frame['target'].values.reshape(-1, 1)

jumlah_epochs = 1000
model.fit(data_latih, target_latih, epochs=jumlah_epochs)

for layer in model.layers:
    if hasattr(layer, 'weights'):
        print(f"Layer: {layer.name}")
        weights, biases = layer.get_weights()
        print(f"Weights:\n{weights}")
        print(f"Biases:\n{biases}")
