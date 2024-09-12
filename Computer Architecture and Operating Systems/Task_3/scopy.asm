; Program przyjmuje dwa parametry będące nazwami plików kolejno '*.in' i '*.out'.
; Dla każdego odczytanego z pliku *.in bajtu, którego wartość jest kodem ASCII litery 's' lub 'S' zapisuje ten bajt do pliku *.out.
; Dla każdego odczytanego z pliku *.in maksymalnego niepustego ciągu bajtów niezawierającego bajtu, którego wartość jest kodem ASCII litery s lub S, 
; zapisuje do pliku *.out 16-bitową liczbę zawierającą liczbę bajtów w tym ciągu modulo 65536.
; Liczbę tę zapisuje binarnie w porządku cienkokońcówkowym.


%macro check_for_errors 0 ; Makro sprawdzające, czy podczas wywołania funkcji nastąpił błąd.
    test rax, rax
    js error              ; Skok jeśli w 'rax' jest ujemna wartość.
%endmacro


global _start
_start:
    BUFFER_SIZE equ 16384             ; Bufor ma rozmiar 16384 bajtów.
    SYS_OPEN equ 2                    ; Nr. funkcji systemowej otwierającej/tworzącej plik.
    SYS_WRITE equ 1                   ; Nr. funkcji systemowej zapisującej do pliku.
    SYS_CLOSE equ 3                   ; Nr. funkcji systemowej zamykającej plik.
    SYS_EXIT equ 60                   ; Nr. funkcji systemowej kończącej program.
    OPEN_OR_CREATE_FLAG equ 301o      ; Nr. odpowiedniej flagi potrzebnej podczas tworzenia pliku (O_WRONLY(1o) | O_CREAT(100o) | O_EXCL(200o)  = 301o), gdzie 'o' oznacza liczbę w zapisie oktalnym.
    SMALL_S_ASCII_CODE equ 115
    LARGE_S_ASCII_CODE equ 83
    CORRECT_NUMBER_OF_ARGUMENTS equ 3 ; Oczekiwana liczba argumentów to 3, ponieważ pierwszym jest jeszcze nazwa programu.
    CREATED_FILE_PERMISSIONS equ 644o ; Uprawienia to '-rw-r--r--'.


section .bss
    buffer_read resb BUFFER_SIZE      ; Miejsce na zapis 'BUFFER_SIZE' odczytanych znaków.
    buffer_write resb BUFFER_SIZE * 2 ; Miejsce na zapis '2 * BUFFER_SIZE' znaków (tekst do zapisu do pliku może być dłuższy niż odczytany tekst).

section .text

    ;1. Sprawdzenie, czy liczba podanych parametrów jest poprawna.
    sub qword[rsp], CORRECT_NUMBER_OF_ARGUMENTS; Liczba parametrów jest na stosie.
    jnz error


    ;2. Tworzenie i otwieranie pliku do zapisu.
    push SYS_OPEN               ; 'push' i 'pop' zajmuje dwa bajty mniej niż 'mov rax, SYS_OPEN'.
    pop rax
    mov rdi, [rsp + 24]         ; Podajemy wskaźnik na nazwę pliku, do którego będziemy zapisywali (wskaźnik jest na stosie).
    mov rsi, OPEN_OR_CREATE_FLAG
    mov rdx, CREATED_FILE_PERMISSIONS
    syscall
    
    xor r9, r9                  ; 'r9' = '0' jeśli plik do zapisu nie został otworzony i '1' jeśli plik został otworzony.
    check_for_errors            ; Sprawdza wynik działania funkcji i w razie błędu skacze do etykiety 'error'.
    inc r9
    mov r15, rax                ; 'r15' = deskryptor pliku do zapisu.
      
      
    ;3. Otwieranie pliku do odczytu.
    push SYS_OPEN               ; 'push' i 'pop' zajmuje dwa bajty mniej niż 'mov rax, SYS_OPEN'.
    pop rax
    mov rdi, [rsp + 16]         ; Podajemy wskaźnik na nazwę pliku z którego czytamy (wskaźnik jest na stosie).
    xor esi, esi                ; 'O_RDONLY' ma kod '0' i oznacza, że plik jest otwarty tylko do odczytu (wykonanie dla 'esi' kosztuje bajt mniej niż dla 'rsi').
    syscall
    
    xor r13, r13                ; 'r13' = '0' jeśli plik do odczytu nie został otworzony i '1' jeśli plik został otworzony.
    check_for_errors
    inc r13
    mov r14, rax                ; 'r14' = deskryptor pliku do odczytu.

    xor ebx, ebx                ; 'bx' = długość maksymalnego ciągu bez 's' oraz 'S' (liczba 16-bitowa). Zeruję 'ebx', bo taka operacja kosztuje bajt mniej.
    
file_loop:                      ; Pętla, w której odczytywane są kolejne fragmenty z pliku do odczytu.

    ;4. Odczytanie z pliku do bufora.
    xor eax, eax                ; 'SYS_READ' ma kod 0.
    mov rdi, r14                ; Podajemy deskryptor pliku do odczytu.
    mov rsi, buffer_read        ; 'buffer_read' to miejsce do zapisu wczytanych danych.
    push BUFFER_SIZE            ; 'push' i 'pop' zajmuje dwa bajty mniej niż 'mov rdx, BUFFER_SIZE'.
    pop rdx
    syscall

    check_for_errors
    
    or eax, eax                 ; 'rax' = liczba bajtów wczytanych z pliku do bufora. Ta liczba zmieści się w 'eax', więc wystarczy sprawdzić tylko ten rejestr.
    jz end_of_file              ; Skok jeśli wczytane zostało 0 bajtów, czyli plik się skończył.


    ;5. Analiza wczytanych danych.
    xor r8, r8                  ; 'r8' = iterator pętli po buforze.
    xor edx, edx                ; 'edx' = indeks pierwszego wolnego miejsca w buforze do zapisu.

buffer_loop:                    ; Pętla po każdym bajcie zapisanym do bufora.
    mov cl, [buffer_read + r8]  ; Wczytanie obecnego bajtu z bufora.
    
    cmp cl, SMALL_S_ASCII_CODE  ; Sprawdzenie, czy obecny bajt jest kodem ASCII litery 's' lub 'S'.
    je character_s_or_S
    cmp cl, LARGE_S_ASCII_CODE
    je character_s_or_S
    
    inc bx          ; Jeśli wczytany bajt jest różny od 'S' i 's' to zwiększamy długość ciągu bez tych liter.
    setz r12b       ; Ustawia 'r12b' na wartość 'ZF', czyli na '1' jeśli 'bx = 0' i na '0' w przeciwnym przypadku.
    
    jmp not_s_or_S


character_s_or_S:       ; Jeśli trafiamy na 'S' lub 's', to najpierw zapisujemy do pliku długość ciągu bez tych liter, chyba że ta była zerowa.  
    or r12b, r12b       ; Przypadek brzegowy, kiedy ciąg ma długość 65536, czyli 0 po przyłożeniu modulo.
    jnz non_zero_length ; Wystarczy sprawdzić, czy nastąpiło przeniesienie po ostatnim zwiększaniu długości ciągu, jeśli tak, to dopisujemy długość.
    
    or ebx, ebx         ; Sprawdzamy, czy długość ciągu bez 's' i 'S' jest zerowa.
    jz zero_length

non_zero_length:
    xor r12b, r12b                     ; Zerujemy przeniesienie po ostatnim zwiększaniu długości ciągu.

    mov [buffer_write + rdx], bx       ; Zapisujemy do bufora wynikowego młodszą część liczby oznaczającej długość ciągu bez 's' i 'S'.

    xor ebx, ebx                       ; Zerujemy długość ciągu bez 'S' i 's'. Zerowanie 'ebx' zamiast 'bx' kosztuje bajt mniej.
    add edx, 2                         ; Przesuwamy wolne miejse w buforze o dwa.

zero_length:                           ; Dopisujemy do wyniku wartość ASCII liter 'S' lub 's'.
    mov cl, [buffer_read + r8]         ; 'cl' = obecny bajt ('S' lub 's').
    mov [buffer_write + rdx], cl       ; Dopisujemy 's' lub 'S' do tekstu, który zapiszemy do pliku.
    inc edx                            ; Przesuwamy wolne miejse w buforze do zapisu.

not_s_or_S:

    ; Obsługa pętli 'buffer_loop'. Skaczemy tyle razy, ile elementów zostało wczytanych z pliku do bufora ('rax').
    inc r8
    cmp r8, rax
    jb buffer_loop


write_to_file:

    ;6. Zapisanie bufora wynikowego do pliku.
    push SYS_WRITE                  ; 'push' i 'pop' zajmuje dwa bajty mniej niż 'mov rax, SYS_WRITE'.
    pop rax
    mov rdi, r15                    ; Podajemy deskryptor pliku do zapisu.
    mov rsi, buffer_write           ; Wskaźnik na miejsce, z którego mają być przepisywane dane.
    syscall                         ; Liczba bajtów do zapisania jest już poprawnie ustawiona w rejestrze 'rdx'. 

    check_for_errors
    
    mov rsi, [buffer_write + rax]   ; Przesuwamy 'buffer_write' o tyle ile z niego zapisaliśmy, żeby w razie potrzeby nie zapisywać dwa razy tego samego.
    mov [buffer_write], rsi         ; Aktualizujemy 'buffer_write'.
    sub edx, eax                    ; Sprawdzamy, czy wszystkie bajty, które miały być zapisane, rzeczywiście zostały zapisane.
    jnz write_to_file               ; Jeśli zostały to zapisujemy ponownie.
    
    jmp file_loop                   ; Powtarzamy, aż plik się nie skończy.


end_of_file:

    ;7. Koniec pliku. Może sie zdażyć, że plik nie kończył się na 'S' lub 's', więc na koniec ręcznie dopisujemy długość ciągu ('bx'), o ile ta jest niezerowa.
    or r12b, r12b              ; Przypadek brzegowy, kiedy ciąg ma długość 65536, czyli 0 po przyłożeniu modulo.
    jnz non_zero_length_eof    ; Wystarczy sprawdzić, czy nastąpiło przeniesienie po ostatnim zwiększaniu długości ciągu, jeśli tak, to dopisujemy długość.
    
    or ebx, ebx                ; Sprawdzamy, czy długość ciągu bez 's' i 'S' jest zerowa.
    jz no_errors

non_zero_length_eof:
    xor r12b, r12b             ; Zerujemy przeniesienie.
    
    mov [buffer_write], bx     ; Zapisujemy do bufora wynikowego młodszą część liczby oznaczającej długość ciągu bez 's' i 'S'.

    xor ebx, ebx               ; Zerujemy długość ciągu bez 's' i 'S'. Zerowanie 'ebx' zamiast 'bx' kosztuje bajt mniej.
    push 2                     ; 'push' i 'pop' zajmuje dwa bajty mniej niż 'mov rdx, 2'.
    pop rdx                    ; '2' to liczba bajtów, które mają zostać zapisane do pliku wynikowego (potrzebne po skoku).             
    jmp write_to_file


;8. Koniec programu.    
error:

    or r8, -1; W przypadku błędu, chcemy żeby bit znaku 'r8' był zapalony, bo to on będzie wynikiem działania programu.
    
no_errors:
    
    ;9. Zamykanie pliku do odczytu.
    
    or r13, r13         ; Sprawdzamy, czy plik do odczytu został otworzony.
    jz plik_odczyt_nieotwarty
    push SYS_CLOSE      ; Zamiast 'mov rax, SYS_CLOSE'.
    pop rax
    mov rdi, r14        ; Podajemy deskryptor pliku do odczytu.
    syscall

    or r8, rax          ; Jeśli był błąd, czyli wartość w 'rax' jest ujemna, to zostanie zapalony bit znaku 'r8'.
plik_odczyt_nieotwarty:

    ;10. Zamykanie pliku do zapisu.
    
    or r9, r9           ; Sprawdzamy, czy plik do zapisu został otworzony.
    jz plik_zapis_nieotwarty
    push SYS_CLOSE      ; Zamiast 'mov rax, SYS_CLOSE'.
    pop rax
    mov rdi, r15        ; Podajemy deskryptor pliku do zapisu.
    syscall

    or r8, rax          ; Jeśli był błąd, czyli wartość w 'rax' jest ujemna, to zostanie zapalony bit znaku 'r8'. 
plik_zapis_nieotwarty:
      
    shr r8, 63          ; Chcemy znać tylko bit znaku 'r8', czyli '1' jeśli był błąd i '0' jeśli błędu nie było.
    
    push r8             ; 'push' i 'pop' zajmuje dwa bajty mniej niż 'mov rdi, r8'.
    pop rdi             ; Wynik działania programu jest w 'rdi'.
    push SYS_EXIT       ; Zamiast 'mov rax, SYS_EXIT'.
    pop rax
    syscall
