cam = webcam(1);
cam.Brightness = 0;
preview(cam);

alpha = 0.9;
while true
I1 = rgb2gray(snapshot(cam));
I2 = rgb2gray(snapshot(cam));
Ht = (alpha.*I1)+((1-alpha).*I2);
Silhouette = abs(rgb2gray(snapshot(cam))-Ht);
imagesc(Silhouette>40);colormap gray;

end