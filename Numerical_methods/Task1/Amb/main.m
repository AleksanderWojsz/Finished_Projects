N = 100;
c = 1;
function retval = f (x, y)
    retval = x;
endfunction


[A, b, S] = generate_A_and_b_and_S(N, c, @f); % '@' Pozwala na przekazanie funkcji
solution = preconditioning_conjugate_gradient(A, b, S, 1e-03, 1000);
draw_plot(solution, N);

clear % Usunięcie wszystkich zmiennych z pamięci
