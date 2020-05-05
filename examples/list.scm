;; List functions
(define l '(1 2 3 4))

(car l)
(cdr l)
(cons 10 l)
(append l '(5 6 7 8))

(filter (lambda (x) (= (mod x 2) 0)) l)
(define square (lambda (x) (* x x)))
(map square l)

;; More primitives supported in README