# lab3

### 221900073 孙佳琪



## CNN_knowledge

#### 二维互相关运算

**二维互相关运算**是卷积神经网络（CNN）中的一种基本操作，用于从输入数据（如图像）中提取特征。卷积窗口从输入张量的左上角开始，从左到右、从上到下滑动。 当卷积窗口滑动到新一个位置时，包含在该窗口中的部分张量与卷积核张量进行按元素相乘，得到的张量再求和得到一个单一的标量值，由此我们得出了这一位置的输出张量值。

**二维互相关运算**与**卷积**的区别在于：

- **互相关运算**：直接使用卷积核，不翻转。
- **卷积运算**：在数学定义中，卷积核需要先进行**水平翻转和垂直翻转**，然后再进行滑动计算。



#### 填充和步幅

- **填充**：通过在输入张量的边缘添加额外的像素，可以控制输出张量的尺寸，并保留更多的边缘信息。
- **步幅**：通过调整卷积核的滑动步长，可以降低输出张量的尺寸，从而减少计算量和内存消耗。

- **综合应用**：填充和步幅可以结合使用，以灵活控制卷积操作的输出形状。

​	在文件所展示的例子中，输入张量 X 的形状为 8×8，卷积核大小为 3×5，填充为（0, 1），步幅为（3, 4）。填充后，输入张量的形状变为 8×10，经过卷积操作后，输出形状为 2×2。



#### 多输入输出通道

在实际的CNN中，输入数据通常具有多个通道（如彩色图像的3个通道），而卷积层也会输出多个通道以提取丰富的特征。

- **多输入通道**：处理具有多个通道的输入数据（如彩色图像），卷积核的通道数必须与输入通道数一致。
- **多输出通道**：通过多个卷积核提取不同的特征，输出通道的数量决定了卷积层的表达能力。
- **综合应用**：多输入输出通道是CNN处理复杂数据和提取丰富特征的基础，广泛应用于图像分类、目标检测等任务。



#### 汇聚层

**汇聚层**是卷积神经网络（CNN）中的重要组成部分，主要用于降低特征图的空间尺寸（即宽度和高度），从而减少计算量并增强模型对输入数据的平移不变性。汇聚层通常不包含可学习的参数，而是通过固定的操作（如取最大值或平均值）对输入数据进行下采样。

- **汇聚层的作用**：降低特征图的空间尺寸，增强平移不变性，减少计算量。
- **汇聚层的类型**：最大汇聚和平均汇聚。
- **填充与步幅**：通过填充和步幅控制输出形状。
- **多通道输入**：汇聚层对每个输入通道单独处理，输出通道数与输入通道数相同。



#### 补全训练部分的代码实现

​	使用均方误差（MSE）作为损失函数，衡量模型输出与目标输出之间的差异。使用随机梯度下降（SGD）优化器来更新卷积核的参数。学习率设置为0.1，控制参数更新的步长。

​	在每一个训练轮次中：

- **前向传播**：通过卷积层计算输出 `output`。
- **计算损失**：使用均方误差损失函数计算模型输出与目标输出之间的差异。
- **清除梯度**：在每次反向传播之前，清除之前的梯度。
- **反向传播**：计算损失对模型参数的梯度。
- **更新参数**：使用优化器更新卷积核的参数。
- **打印损失**：每2个epoch打印一次损失值，方便观察训练过程。

![image-20250307200734060](C:\Users\sjq\Desktop\college\深度学习及其应用\lab3\lab3\image-20250307200734060.png)

最终的输出结果如图所示，较为接近torch.tensor([[1.0, -1.0]])。

为了使训练结果接近目标，多次调整了训练次数和学习率，最后设置训练100轮，学习率为0.1。



## CNN_main

#### 在CIFAR数据集上实现CNN

​	定义CNN网络：

- **卷积层**：
  - `conv1`：输入通道数为3（RGB图像），输出通道数为32，卷积核大小为3x3，padding=1，保持输出特征图大小不变。
  - `conv2`：输入通道数为32，输出通道数为64，卷积核大小为3x3，padding=1。
- **池化层**：
  - 使用最大池化层 `MaxPool2d`，池化窗口大小为2x2，步幅为2，将特征图大小减半。
- **全连接层**：
  - `fc1`：将卷积层的输出展平后输入到全连接层，输入大小为 `64 * 8 * 8`（64个通道，8x8的特征图），输出大小为512。
  - `fc2`：输入大小为512，输出大小为10（对应CIFAR-10的10个类别）。
- **Dropout层**：
  - 在全连接层后加入Dropout层，dropout概率为0.5，用于防止过拟合。

​	训练过程：

- **`optimizer.zero_grad()`**：清除优化器中之前的梯度，避免梯度累积。
- **`outputs = model(inputs)`**：将输入数据传入模型，得到预测输出。
- **`loss = criterion(outputs, labels)`**：计算预测输出与真实标签之间的损失值。

- **`loss.backward()`**：计算损失函数关于模型参数的梯度（反向传播）。
- **`optimizer.step()`**：根据梯度更新模型参数。

​	最终训练结果如图所示：

![image-20250307204306809](C:\Users\sjq\Desktop\college\深度学习及其应用\lab3\lab3\image-20250307204306809.png)

![image-20250307204324561](C:\Users\sjq\Desktop\college\深度学习及其应用\lab3\lab3\image-20250307204324561.png)



### 在MNIST数据集上实现CNN

LeNet模型的定义：

LeNet 模型的结构如下：

- **输入**：28x28 的灰度图像（MNIST 数据集）。
- **卷积层 1**：
  - 输入通道数：1（灰度图像）。
  - 输出通道数：6。
  - 卷积核大小：5x5。
  - 激活函数：Sigmoid。
- **池化层 1**：
  - 池化类型：平均池化（`AvgPool2d`）。
  - 池化窗口大小：2x2。
  - 步幅：2。
- **卷积层 2**：
  - 输入通道数：6。
  - 输出通道数：16。
  - 卷积核大小：5x5。
  - 激活函数：Sigmoid。
- **池化层 2**：
  - 池化类型：平均池化（`AvgPool2d`）。
  - 池化窗口大小：2x2。
  - 步幅：2。
- **全连接层 1**：
  - 输入大小：16x5x5（16个通道，5x5的特征图）。
  - 输出大小：120。
  - 激活函数：Sigmoid。
- **全连接层 2**：
  - 输入大小：120。
  - 输出大小：84。
  - 激活函数：Sigmoid。
- **输出层**：
  - 输入大小：84。
  - 输出大小：10（对应 MNIST 的 10 个类别）。

训练结果如图所示：

![image-20250307205351533](C:\Users\sjq\Desktop\college\深度学习及其应用\lab3\lab3\image-20250307205351533.png)