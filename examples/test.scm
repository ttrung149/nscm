;;===================================================
;; Sample test scheme code
;;===================================================

; Sample comment

;; Define
(define pi 3.14)
(define circle-area (lambda (r) (* pi (* r r))))
(define rect-area (lambda (x y) (* x y)))

;; Expect 314
(circle-area 10)

;; Set pi = 2, expect pi = 2
(set! pi 2)
pi

;; Function call. Expect 45
((((lambda (x) (lambda (y) (lambda (z) (+ x y z)))) 10) 15) 20)

;; Recursion - factorial function. expect 3628800
(define fact (lambda (n) (if (<= n 1) 1 (* n (fact (- n 1))))))
(fact 10)

;; Recursion - fibonacci function. expect 89
(define fib (lambda (n) (if (< n 2) 1 (+ (fib (- n 1)) (fib (- n 2))))))
(fib 10)

;; Math functions
(sin 3.14159)
(cos 3.14159)

;; Control flow. expect 15
(if (> 2 3) 10 15)
