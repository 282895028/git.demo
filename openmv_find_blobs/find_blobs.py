# 这个例子展示了使用OpenMV Cam的单色RGB565跟踪。
import sensor, image, time, math

threshold_index = 2 # 0 for red, 1 for green, 2 for blue

# 颜色跟踪阈值 (L Min, L Max, A Min, A Max, B Min, B Max)
# 下面的阈值通常跟踪红色/绿色/蓝色的东西。您可能希望调整它们……
thresholds = [(16, 8, -39, 18, -31, 9), # generic_red_thresholds
              (54, 77, -9, 9, 85, 47), # generic_green_thresholds
              (0, 65, -21, 28, -128, -25)]     # generic_blue_thresholds

sensor.reset()#重置感光元件，重置摄像机
sensor.set_pixformat(sensor.RGB565) #设置颜色格式为RGB565，彩色，每个像素16bit。
sensor.set_framesize(sensor.QVGA)   #图像大小为QVGA
sensor.skip_frames(time = 2000)     #跳过n张照片，在更改设置后，跳过一些帧，等待感光元件变稳定。
sensor.set_auto_gain(False)         #颜色识别必须关闭自动增益，会影响颜色识别效果
sensor.set_auto_whitebal(False)     #颜色识别必须关闭白平衡，会影响颜色识别效果，导致颜色的阈值发生改变
clock = time.clock()

# 只有像素大于“pixels_threshold”和面积大于“area_threshold”的区域才是
# 由下面的"find_blobs"返回。更改“pixels_threshold”和“area_threshold”
# 相机的分辨率。"merge=True"合并图像中所有重叠的斑点。

while(True):
    clock.tick()# 追踪两个snapshots()之间经过的毫秒数.
    img = sensor.snapshot()#截取感光元件中的一张图片
    #在img.find_blobs这个函数中，我们进行颜色识别
    #roi是“感兴趣区”，是在画面的中央还是右上方或者哪里进行颜色识别。此处我们没有进行配置，默认整个图像进行识别
    for blob in img.find_blobs([thresholds[threshold_index]], pixels_threshold=200, area_threshold=200, merge=True):
        # 这些值依赖于blob不是循环的-否则它们将不稳定。
        #if blob.elongation() > 0.5:
            #img.draw_edges(blob.min_corners(), color=(255,0,0))    #利用一个红色的方框，绘制出Blob的最小边界
            #img.draw_line(blob.major_axis_line(), color=(0,255,0)) #利用一个绿色的线，绘制穿过最小面积矩形的最长边
            #img.draw_line(blob.minor_axis_line(), color=(0,0,255)) #利用一个蓝色的线，绘制穿过最小面积矩形的最短边
        # 这些值始终是稳定的。
        img.draw_rectangle(blob.rect())      #用矩形标记出目标颜色区域
        img.draw_cross(blob.cx(), blob.cy()) #在目标颜色区域的中心画十字形标记
        # 注意- blob的旋转是唯一的0-180。
        #img.draw_keypoints([(blob.cx(), blob.cy(), int(math.degrees(blob.rotation())))], size=20)
    #print(clock.fps()) #打印帧率
