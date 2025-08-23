function draw_plot (solution, N)
  solution = reshape(solution, N-1, N-1);
  solution = [zeros(1, size(solution, 2)); solution; zeros(1, size(solution, 2))];
  solution = [zeros(size(solution, 1), 1) solution zeros(size(solution, 1), 1)];

  axis_x = linspace(0, 1, N+1);
  axis_y = linspace(0, 1, N+1);
  mesh(axis_x, axis_y, solution');
  xlabel "x";
  ylabel "y";
  zlabel "u";
endfunction
