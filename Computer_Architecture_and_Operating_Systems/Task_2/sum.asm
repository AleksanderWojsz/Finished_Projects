global sum
section .text

sum:
    ;0. Główna pętla (pomija pierwszy element)
    mov r8, 1; i = 1 (r8 to iterator pętli)
petla_glowna:


    ;1. Liczenie potęgi
potega:
    mov rax, 64; rax = potęga = 64
    mul r8; potęga*=i
    mul r8; potęga*=i
    div rsi; potęga/=n


    ;Możliwe, że liczbę którą dodajemy trzeba będzie rozdzielić na dwie części i każdą z nich dodać do sąsiadujących miejsc w tablicy
    ;2. Przechodzimy do miejsca w tablicy gdzie będziemy dodawali
    mov rcx, rax; rcx = potęga
    shr rcx, 6; rcx = potęga/64
    lea r10, [rdi + rcx * 8]; r10 = &x[potega/64] (r10 = adres komórki w tablicy, gdzie będziemy dodawać)


    ;3. Komórkę tablicy którą dodajemy wypełniamy bitem znaku dotychczasowego wyniku
    mov r11, [rdi + r8 * 8]; r11 = kopia obecnie dodawanej liczby

    mov rdx, [rdi + r8 * 8 - 8]; rdx = najbardziej znacząca komórka z części tablicy w której już jest wynik
    sar rdx, 63; rdx = rejestr w którym powielony jest znak aktualnego wyniku (rdx = 0000... lub 1111...)
    mov [rdi + r8 * 8], rdx; nadpisanie obecnie dodawanej komórki powielonym bitem znaku aktualnego wyniku


    ;4. Dodajemy do "lewej", mniej znaczącej części tablicy
    ;Korzystamy z 32 bitowych części rejestrów bo jest taniej
    mov ecx, eax; ecx = potega
    and ecx, 63; ecx = potega % 64
    mov rdx, r11; rdx = obecnie dodawana komórka tablicy
    shl rdx, cl; logiczny shift w lewo o potega % 64 (odpowiednio przesuwamy obecną komórkę, żeby móc ją dodać)

    xor r9, r9; zerujemy przeniesienie
    add [r10], rdx; dodajemy do lewej części
    adc r9, 0; zapamietujemy przeniesienie


    ;5 Dodajemy do "prawej", bardziej znaczącej części tablicy
    add r10, 8; przesuwamy się z dodawaniem o jedno miejsce dalej w tablicy
    
    ;Korzystamy z 32 bitowych części rejestrów bo jest taniej
    mov edx, ecx; edx = potega % 64
    mov ecx, 64
    sub ecx, edx; ecx = 64 - potega % 64

    mov rdx, r11; rdx = obecny blok

    cmp rcx, 64; 'sar' nie działa dla przesunięcia równego 64. Wtedy przesuwamy na dwa razy
    jne nie_64
    sar rdx, 1;
    sar rdx, 63;

nie_64:
    sar rdx, cl; przesunięcie arytmetyczne w prawo o 64 - potega % 64

    add r9, -1; ustawia flagę CF na wartość rejestru r9 oznaczającego przeniesienie (wartośc 0 lub 1)

    adc [r10], rdx; dodajemy do "prawej", bardziej znaczącej części tablicy z przeniesieniem

    ;6 Do każdej następnej komórki tablicy, aż do obecnie dodawanej włącznie, dodajemy komórkę wypełnioną powielonym bitem znaku dodawanej liczby razem z przeniesieniem
    lea rdx, [rdi + r8 * 8]; rdx = adres obecnie dodawanej komórki
    jmp pierwsze_przejscie; przed pierwszym wykonaniem sprawdzamy warunek if'a

petla_dopisanie:
    sar r11, 63; rdx = rejestr w którym powielony jest znak obecnie dodawanej liczby (rdx = 0000... lub 1111...)

    add r9, -1; ustawia flagę CF na wartość rejestru r9 oznaczającego przeniesienie (wartośc 0 lub 1)

    adc [r10], r11; dodajemy z przeniesieniem

pierwsze_przejscie:
    ; obsługa pętli dopisującej
    mov r9, 0; zerujemy przeniesienie
    adc r9, 0; zapamietujemy przeniesienie
    add r10, 8; przesuwamy blok do którego będziemy dodawali
    cmp rdx, r10; rdx = najbardziej znacząca komórka wyniku, r10 = komórka gdzie będziemy dodawali. Dodajemy, tak długo jak nie wychodzimy poza aktualny wynik.
    jns petla_dopisanie; skok jak wynik nieujemny


    ; obsługa pętli głównej
    inc r8 
    cmp rsi, r8; n - i     
    jg petla_glowna; skok jak n - i >= 0

    ret  
