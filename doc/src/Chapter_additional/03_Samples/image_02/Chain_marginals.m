
clear; clc;
close all;

d = [2,5,8];
figure;
hold on;
grid on;
for k=1:length(d)
    temp = load(['./Chain_marginals/Dom_', num2str(d(k)), '.txt']);
    Data(:,k) = temp(:,1);
    labels{k} = ['$\left| Dom(Y_i) \right| = ', num2str(d(k)), '$'];
end
plot(2:10, Data, 'o-','linewidth', 2);
ylabel('$P(Y_n = 0 | Y_0 = 0)$', 'Interpreter', 'latex');
xlabel('Chain size', 'Interpreter', 'latex');
l = legend(labels);
set(l, 'interpreter', 'latex');
set(gca,'FontSize',20);