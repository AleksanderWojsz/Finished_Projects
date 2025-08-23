N = 100;
c = 1;
function retval = f (x, y)
    retval = x;
endfunction
delta = 1e-03
epsilon = 1e-03


[A, b] = generate_A_and_b(N, c, @f); % '@' Pozwala na przekazanie funkcji
solution = conjugate_gradient(A, b, delta, epsilon, 1000);
draw_plot(solution, N);

clear % Usunięcie wszystkich zmiennych z pamięci
