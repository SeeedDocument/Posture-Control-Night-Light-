# Posture Control Night Light

最近的睡眠不是很好，发现主要问题在于每次困了，一起身去关灯困意就完全没有了。于是，我去互联网上浏览了一下，最终得到了一个非常棒的创意。我准备制作一个可以定时关闭的姿势灯，因为这样的话，我就可以不需做任何动作就能关闭灯。当然也不能每次都通过定时去关灯，这样的话就太局限了，所以还需要设计一个不依赖定时就能马上关闭灯的功能。但是，我还是感觉功能还是不够，最后我准备设计出可以控制类似手机摇一摇的控制，当然我设想的功能只是摇一摇灯就能改变灯的颜色。为了使外型简单我准备不用按键去控制灯的定时以及开关功能，直接采用传感的方式来控制，比如说我想定时5分钟，那么我将姿势灯以桌子和姿势灯的接触点为原点顺时针旋转90°。假如我想关闭姿势灯以桌子和正常摆放的姿势灯的接触点为原点顺时针旋转180°。当然，变换颜色就很简单只需要摇一摇姿势灯就能变换颜色了。

## 硬件构成

### Seeeduino Nano

主控制选择的是[Seeeduino Nano](https://www.seeedstudio.com/Seeeduino-Nano-p-4111.html) 之所以选择它就是因为它小而且便宜。当然，为了更方便接线，直接购买[Grove Shield for Arduino Nano](https://www.seeedstudio.com/Grove-Shield-for-Arduino-Nano-p-4112.html)也是一个很好的选择。

![](https://github.com/hansonCc/Arduino/raw/master/pose_light/DOC/Grove%20Shield%20for%20Arduino%20Nano.png)

### Grove - 3-Axis Digital Accelerometer ±200g (ADXL372)

[Grove - 3-Axis Digital Accelerometer ±200g (ADXL372)](https://www.seeedstudio.com/Grove-3-Axis-Digital-Accelerometer-200g-ADXL372-p-4003.html)是一种超低功耗数字输出MEMS加速度计，它可以提供100位/ LSB比例因子的12位输出。该传感器最显着的特点是其超低功耗（测量模式下仅为22μA）和大测量范围(±200g)。所有数据都通过Grove I2C端口输出,i2C地址可以更改。为了满足更广泛的测量需求，采样率可以选择400Hz/ 800Hz/1600Hz/3200Hz/6400Hz，带宽可以选择200Hz/400Hz/800Hz/1600Hz/3200Hz。除了用作加速度测量之外，您还可以使用此模块进行冲击和冲击检测。所以选用它来检测姿势灯的姿势是非常合适的。

![](https://github.com/hansonCc/Arduino/raw/master/pose_light/DOC/3-Axis%20Digital%20Accelerometer.png)

### Grove - RGB LED Ring (20 - WS2813 Mini)

为了达到良好的灯效，我选择了[The Grove - RGB LED Ring (20 - WS2813 Mini)](http://wiki.seeedstudio.com/Grove-LED_ring)，因为每个LED都有一个恒定电流驱动器，因此即使电压发生变化，颜色也会非常一致。这样灯效就会变的很合适。

![](https://github.com/hansonCc/Arduino/raw/master/pose_light/DOC/Grove%20-%20RGB%20LED%20Ring.jpg)

### Li-po Rider

Seeeduino Nano 支持5V供电 ，但是为了使姿势灯方便携带显然一直使用电脑USB供电是一个不太合适的选择，这里我们选用[Li-po Rider](https://www.seeedstudio.com/Li-po-Rider-p-710.html)来解决这个问题。

![](https://github.com/hansonCc/Arduino/raw/master/pose_light/DOC/Li-po%20Rider.jpg)

## 软件设计

### Grove - 3-Axis Digital Accelerometer ±200g (ADXL372)

Grove - 3-Axis Digital Accelerometer ±200g 有两种模式一种是普通读写模式，还以一种是FIFO读写模式。非常感谢seeed提供的[wiki](http://wiki.seeedstudio.com/Grove-3-Axis_Digital_Accelerometer_200g-ADXL372/)，因为里面很完美的库文件，使我开发这个小设计节省不少时间。最终我选择fifo模式输出一组数据然后对其求平均数。

```c
  uint16_t samples = acc.samples_in_fifo();
  // To ensure that data is not overwritten and stored out of order,
  // at least one sample set must be left in the FIFO after every read
  if (samples > 12) {
    samples = (samples > BUFFER_SIZE)? BUFFER_SIZE : (samples / 6 - 1) * 6;
    acc.fifo_read(buffer, samples);
    for (j=0; j<samples; j+=6) {
      // convert raw data
      xyz_t *xyz = acc.format(buffer + j);
      xyz_acc.x = xyz->x + xyz_acc.x;
      xyz_acc.y = xyz->y + xyz_acc.y;
      xyz_acc.z = xyz->z + xyz_acc.z;
    }
     xyz_acc.x = xyz_acc.x * 6.0 / j;
     xyz_acc.y = xyz_acc.y * 6.0 / j;
     xyz_acc.z = xyz_acc.z * 6.0 / j;
```

### Grove - RGB LED Ring (20 - WS2813 Mini)

关于Grove - RGB LED Ring (20 - WS2813 Mini)我只是简单写了一个控制函数，控制函数里面调用的代码我都是参考[wiki](http://wiki.seeedstudio.com/Grove-LED_ring)上面内容，这是我写的控制函数的代码

```c
void Led_Control(uint32_t Led_num,uint32_t Color,bool Switch)
{
    for(int i=0; i<Led_num; i++)
    {
      if(Switch)
      {
        strip.setPixelColor(i, Color);
        strip.show();
      }  
      else
      {
        strip.setPixelColor(i, Off_All);
        strip.show();
      }
    }
}
```

## 产品整合

最终经过半天的时间我搭建好整体硬件连接，硬件的整体连接如下图：

![](https://github.com/hansonCc/Arduino/raw/master/pose_light/DOC/Overall%20connection%20diagram.jpg)

这是一个非常需要想象力的事情，显然我对3D外壳设计就显得更加的理性化一点，所以还请为各位对于3D外壳的设计少做一些评价。


