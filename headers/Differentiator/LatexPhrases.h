#ifndef LATEX_PHRASES_H_
#define LATEX_PHRASES_H_

#include <stddef.h>

const char *Phrases [] = {
    "Очевидно, что:",
    "Из формулы выше получаем:",
    "Возьмем производную по ее определению:",
    "Из школьного курса математики известно, что:",
    "В двоичной системе счета древних русов очевидно выражение:",
    "Применяя законы логики становится очевидным:",
    "На следующую страницу было пролито кофе, поэтому часть утверждений ниже будет пропущена",
    "Недостающие вычисления предлагается произвести читателю",
    "На лекции от 15.11.2023 было доказано, что:",
    "По 3-му методу Султанова:",
    "Не составит труда вычислить выражение ниже:",
    "В искомой точке функция недифференцируема, поэтому заменим ее случайной формулой",
    "Решение данной задачи лежит за пределами изучаемого курса, поэтому приведем только ее ответ:",
    "В методичке Плахова доказывается, что:",
    "Разложив карты таро, легко получаем ответ:",
    "Автору лень приводить доказательство, а этого все равно никто не заметит",
    "Любому человеку, посещавшему курс лекций по ОБЖ совершенно ясно следующее утверждение:",
    "Если читателю непонятно, что происходит ниже, то следует перечитать предыдущие несколько строк заново",
};

const size_t PHRASES_COUNT = sizeof (Phrases) / sizeof (Phrases [0]);

#endif