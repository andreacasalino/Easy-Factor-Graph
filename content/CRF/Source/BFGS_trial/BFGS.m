function [x , f_story] = BFGS(f, df, x0, iter)

n=length(x0);
x=x0;
inv_B=eye(n);
f_story=f(x);
grad_new = df(x);

for k=1:iter
    grad_f=grad_new;

    p=-inv_B*grad_f;
    alfa = line_search(x, p, f);
    s=alfa*p;
    x=x+s;
    f_story=[f_story, f(x)];
    
    grad_new=df(x);
    y=grad_new - grad_f;
    
    Temp=eye(n) - (1/(s'*y)) * s*y';
    inv_B = Temp*inv_B*Temp' + (1/(s'*y)) * s*s';
    eig_inv_B = eig(inv_B);
    
    if(min(abs(eig_inv_B)) < 1e-5)
        inv(inv_B)
        return;
    end
    
    if(norm(grad_new) <= 1e-3)
        inv(inv_B)
        return;
    end
end


end

function alfa = line_search(x, p, f)

f_old=f(x);
alfa=1;
for k=1:5
    alfa=alfa*0.5;
    f_new = f(x + p*alfa);
    if(f_new > f_old)
        return;
    end
    f_old=f_new;
end

end