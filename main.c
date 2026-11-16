/**
 * @file main.c
 * @brief Лабораторна робота №5: Програмні потоки для роботи з файлами.
 *
 * Програма зчитує вхідні параметри табулювання з текстового файлу,
 * обчислює значення функції f(x) = 0.4*(x-100)^3 + 0.3*x^2 - 0.2*x + 200,
 * формує звіт у текстовому файлі та серіалізує результати у бінарний файл.
 * а також зчитує бінарні дані у динамічний масив для виведення в консоль.
 * Підтримує коректну роботу з кирилицею у кодуванні UTF-8 (включно з UTF-8 BOM).
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <locale.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define MAX_STR_LEN 100

/**
 * @brief Налаштовує консоль для коректного відображення української мови (UTF-8)
 *        зі збереженням стандартного роздільника дробової частини (крапки).
 */
void init_utf8_console(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    /* Гарантуємо, що десятковим роздільником для чисел є крапка (95.0) */
    setlocale(LC_NUMERIC, "C");
}

/**
 * @brief Пропускає маркер послідовності байтів (UTF-8 BOM), якщо він є на початку файлу.
 * @param file Вказівник на відкритий файловий потік.
 */
void skip_utf8_bom(FILE *file) {
    unsigned char bom[3] = {0};
    if (fread(bom, 1, 3, file) == 3) {
        if (bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
            return; /* BOM знайдено і пропущено */
        }
    }
    /* Якщо BOM відсутній — повертаємо вказівник на початок файлу */
    rewind(file);
}

/**
 * @brief Обчислює значення математичної функції f(x) згідно із завданням.
 * @param x Значення аргументу функції.
 * @return Значення функції f(x).
 */
double calculate_function(double x) {
    return 0.4 * pow(x - 100.0, 3) + 0.3 * pow(x, 2) - 0.2 * x + 200.0;
}

/**
 * @brief Видаляє символи перенесення рядка (\n, \r) з кінця рядка.
 * @param str Вказівник на рядок символів.
 */
void trim_newline(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[--len] = '\0';
    }
}

/**
 * @brief Зчитує дані з бінарного файлу у динамічний масив розміром 2*N
 *        та виводить їх у вигляді таблиці на екран.
 * @param filename Ім'я бінарного файлу для читання.
 */
 void read_and_display_binary(const char *filename) {
    FILE *in_bin = fopen(filename, "rb");
    if (in_bin == NULL) {
        perror("Помилка відкриття бінарного файлу для читання");
        return;
    }

    int n = 0;
    /* Зчитування першого значення — кількості точок N */
    if (fread(&n, sizeof(int), 1, in_bin) != 1 || n <= 0) {
        fprintf(stderr, "Помилка зчитування кількості точок з бінарного файлу.\n");
        fclose(in_bin);
        return;
    }

    /* Виділення пам'яті під динамічний масив розміром 2*N типу double */
    size_t array_size = (size_t)(2 * n);
    double *data_array = (double *)malloc(array_size * sizeof(double));
    if (data_array == NULL) {
        fprintf(stderr, "Помилка виділення динамічної пам'яті.\n");
        fclose(in_bin);
        return;
    }

    /* Зчитування послідовності пар (x, f(x)) у динамічний масив */
    size_t read_count = fread(data_array, sizeof(double), array_size, in_bin);
    fclose(in_bin);

    if (read_count != array_size) {
        fprintf(stderr, "Попередження: зчитано менше елементів (%zu), ніж очікувалося (%zu).\n",
                read_count, array_size);
    }

    /* Виведення таблиці на екран монітора */
    printf("\nДані, зчитані з бінарного файлу (%s):\n", filename);
    printf("+---------------+--------------+------------------+\n");
    printf("|  Номер точки  |  Аргумент x  |  Значення f(x)   |\n");
    printf("+---------------+--------------+------------------+\n");

    for (int i = 0; i < n; i++) {
        double x_val = data_array[2 * i];       /* Парний індекс: аргумент x */
        double fx_val = data_array[2 * i + 1];  /* Непарний індекс: значення f(x) */
        printf("| %13d | %12.4lf | %16.4lf |\n", i + 1, x_val, fx_val);
    }
    printf("+---------------+--------------+------------------+\n");

    /* Звільнення виділеної динамічної пам'яті */
    free(data_array);
}

int main(void) {
    init_utf8_console();

    double x_start, x_end, step;
    int points_count;
    char group_code[MAX_STR_LEN];
    char student_name[MAX_STR_LEN];

    /* 1. Відкриття та зчитування вхідного текстового файлу */
    FILE *in_file = fopen("input.txt", "r");
    if (in_file == NULL) {
        perror("Помилка відкриття вхідного файлу input.txt");
        return EXIT_FAILURE;
    }

    skip_utf8_bom(in_file);

    if (fscanf(in_file, "%lf", &x_start) != 1 ||
        fscanf(in_file, "%lf", &x_end) != 1 ||
        fscanf(in_file, "%d", &points_count) != 1 ||
        fscanf(in_file, "%lf", &step) != 1) {
        fprintf(stderr, "Помилка формату числових даних у файлі input.txt\n");
        fclose(in_file);
        return EXIT_FAILURE;
    }

    /* Очищення буфера від символу нового рядка перед використанням fgets */
    int ch;
    while ((ch = fgetc(in_file)) != '\n' && ch != EOF);

    if (fgets(group_code, MAX_STR_LEN, in_file) == NULL ||
        fgets(student_name, MAX_STR_LEN, in_file) == NULL) {
        fprintf(stderr, "Помилка зчитування рядкових даних (група/студент)\n");
        fclose(in_file);
        return EXIT_FAILURE;
    }
    fclose(in_file);

    trim_newline(group_code);
    trim_newline(student_name);

    /* 2. Відкриття вихідного текстового та бінарного файлів для запису */
    FILE *out_txt = fopen("output.txt", "w");
    if (out_txt == NULL) {
        perror("Помилка створення файлу output.txt");
        return EXIT_FAILURE;
    }

    FILE *out_bin = fopen("data.bin", "wb");
    if (out_bin == NULL) {
        perror("Помилка створення файлу data.bin");
        fclose(out_txt);
        return EXIT_FAILURE;
    }

    /* Запис заголовка таблиці у текстовий файл */
    fprintf(out_txt, "+-------+--------------+------------------+\n");
    fprintf(out_txt, "|   N   |      x       |       f(x)       |\n");
    fprintf(out_txt, "+-------+--------------+------------------+\n");

    /* Підрахунок фактичної кількості точок та запис у бінарний файл */
    int actual_points = 0;
    double temp_x = x_start;
    for (int i = 0; i < points_count && temp_x <= x_end + 1e-9; i++) {
        actual_points++;
        temp_x += step;
    }
    fwrite(&actual_points, sizeof(int), 1, out_bin);

    /* 3. Табулювання функції та паралельний запис у два потоки */
    double current_x = x_start;
    for (int i = 0; i < actual_points; i++) {
        double fx = calculate_function(current_x);

        /* Форматований вивід у текстовий файл */
        fprintf(out_txt, "| %5d | %12.4lf | %16.4lf |\n", i + 1, current_x, fx);

        /* Неформатований запис пари (x, f(x)) у бінарний файл */
        fwrite(&current_x, sizeof(double), 1, out_bin);
        fwrite(&fx, sizeof(double), 1, out_bin);

        current_x += step;
    }

    /* Завершення таблиці та додавання метаданих студента у текстовий файл */
    fprintf(out_txt, "+-------+--------------+------------------+\n");
    fprintf(out_txt, "Група: %s\n", group_code);
    fprintf(out_txt, "Виконав: %s\n", student_name);

    fclose(out_txt);
    fclose(out_bin);

    printf("Дані успішно оброблено та збережено у output.txt і data.bin.\n");

    /* 4. Читання бінарного файлу в динамічний масив та вивід на екран */
    read_and_display_binary("data.bin");

    return EXIT_SUCCESS;
}
