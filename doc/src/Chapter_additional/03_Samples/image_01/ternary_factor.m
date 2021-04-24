clear; clc;
close all;

f=@(a,b) a+2*b;

for ka= 1:4
    for kb = 1:4
        M(ka,kb) = f(ka-1,kb-1);
    end
end
M

%%

clear; clc;
close all;

w_dis = linspace(1.5, 5, 100);

Z_C =@(w)  (4^3 - 4) + 4 * exp(w);
figure;
hold on;
grid on;
plot(w_dis, (4 * exp(w_dis)) ./ Z_C(w_dis), 'b' , 'linewidth', 2);
plot(w_dis, 1 - (4 * exp(w_dis)) ./ Z_C(w_dis), 'r' , 'linewidth', 2);
title('$\Psi_{C,\,V123} = exp (w \cdot \Phi_{C,\,V123}  )$', 'Interpreter', 'latex');
l = legend({'$P( V_{1,2,3} = v)$' , '$1-P( V_{1,2,3} = v)$'});
set(l, 'interpreter', 'latex');
xlabel('$w$', 'Interpreter', 'latex');
set(gca,'FontSize',20);

w_dis = linspace(0.5, 5, 100);

Z_A =@(w)  (4^3 - 4)* exp(w) + 4 ;
figure;
hold on;
grid on;
plot(w_dis, 4 ./ Z_A(w_dis), 'b' , 'linewidth', 2);
plot(w_dis, 1 - 4 ./ Z_A(w_dis), 'r' , 'linewidth', 2);
title('$\Psi_{A,\,V123} = exp (w \cdot \Phi_{A,\,V123}  )$', 'Interpreter', 'latex');
l = legend({'$P( V_{1,2,3} = v)$' , '$1-P( V_{1,2,3} = v)$'});
set(l, 'interpreter', 'latex');
set(gca,'FontSize',20);
xlabel('$w$', 'Interpreter', 'latex');