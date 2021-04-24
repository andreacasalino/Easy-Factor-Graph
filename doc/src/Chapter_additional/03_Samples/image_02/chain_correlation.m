
clear; clc;
close all;

w_dis = linspace(1, 3, 100);

P_B = [ 1 + 0*w_dis; exp(w_dis) ];
P_A =  [ 2*exp(w_dis); 1 + exp(w_dis).^2 ];

for k=1:size(P_B,2)
    P_B(:,k)= 1/(sum(P_B(:,k)))*P_B(:,k);
    P_A(:,k)= 1/(sum(P_A(:,k)))*P_A(:,k);
end

figure;
hold on;
grid on;
plot(w_dis, P_B(1,:), 'b' , 'linewidth', 2);
plot(w_dis, P_B(2,:), 'cyan' , 'linewidth', 2);
plot(w_dis, P_A(1,:), 'r' , 'linewidth', 2);
plot(w_dis, P_A(2,:), 'Color', (1/255)*[255, 149, 138] , 'linewidth', 2);
l = legend({'$P(B=0|C=1)$' ,
    '$P(B=1|C=1)$', 
    '$P(A=0|C=1)$',
    '$P(A=1|C=1)$'});
set(l, 'interpreter', 'latex');
xlabel('$w = \alpha = \beta$', 'Interpreter', 'latex');
set(gca,'FontSize',20);
