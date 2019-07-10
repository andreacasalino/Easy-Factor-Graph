clear; clc;
close all;

c=rand(6,1)*4-2;
f=@(x)  (x-c)'*(x-c);
df=@(x) 2*(x-c);
x0=rand(6,1)*4-2;

[x , f_story] = BFGS(f, df, x0, 100);

figure;
plot(f_story);
grid on;
