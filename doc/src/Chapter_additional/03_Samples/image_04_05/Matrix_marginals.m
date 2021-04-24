
clear; clc;
close all;

temp = load('./Matrix_marginals/w_0_9.txt');
Data(:,1) = temp(:,1);
temp = load('./Matrix_marginals/w_1.txt');
Data(:,2) = temp(:,1);
temp = load('./Matrix_marginals/w_1_1.txt');
Data(:,3) = temp(:,1);

labels={'$weigth = 0.9$','$weigth = 1$','$weigth = 1.1$'};

figure;
hold on;
grid on;
plot(1:size(Data,1), Data, 'o-','linewidth', 2);
ylabel('$P(Y_{i\_i} = 0 | Y_{0\_0} = 0)$', 'Interpreter', 'latex');
xlabel('i', 'Interpreter', 'latex');
l = legend(labels);
set(l, 'interpreter', 'latex');
set(gca,'FontSize',20);