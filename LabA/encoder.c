#include <stdbool.h>
#include <string.h>
#include <stdio.h>

void cyclicFunction(FILE *infile, FILE *outfile, const char *key, int sign);

int main(int argc, char const *argv[])
{
    bool debudgMode = false;
    bool encodingMode = false;
    int sign = 1; // -1 for subtraction, 1 for addition
    char const *key;
    FILE *infile = stdin;
    FILE *outfile = stdout;
    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '+')
        {
            switch (argv[i][1])
            {
            case 'D':
                debudgMode = true;
                continue;

            case 'e':
                encodingMode = true;
                sign = 1;
                key = argv[i] + 2;
                break;

            default:
                break;
            }
        }
        else if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
            case 'e':
                encodingMode = true;
                sign = -1;
                key = argv[i] + 2;
                break;

            case 'i':
                infile = fopen(argv[i] + 2, "r");
                if (infile == NULL)
                {
                    fprintf(stderr, "%s\n", "Error opening input file");
                    return -1;
                }
                break;

            case 'o':
                outfile = fopen(argv[i] + 2, "w");
                if (outfile == NULL)
                {
                    fprintf(stderr, "%s\n", "Error opening output file");
                    return -1;
                }
                break;

            case 'D':
                debudgMode = false;
                continue;

            default:
                break;
            }
        }

        if (debudgMode)
        {
            fprintf(stderr, "%s\n", argv[i]);
        }
    }

    if (encodingMode)
    {
        cyclicFunction(infile,outfile,key,sign);
    }

    fclose(infile);
    fclose(outfile);
    return 0;
}

void cyclicFunction(FILE *infile, FILE *outfile, const char *key, int sign)
{
    int j = 0;
    while (true)
    {
        int digit = fgetc(infile);

        if (digit == -1)
            break;

        if (key[j] == '\0')
            j = 0;

        int result = digit + sign * (key[j] - '0');
        int upperLimit;
        int lowerLimit;
        int delta;
        if (digit >= 48 && digit <= 57)
        {
            upperLimit = 57;
            lowerLimit = 48;
            delta = 10;
        }
        else if (digit >= 65 && digit <= 90)
        {
            upperLimit = 90;
            lowerLimit = 65;
            delta = 26;
        }
        else if (digit >= 97 && digit <= 122)
        {
            upperLimit = 122;
            lowerLimit = 97;
            delta = 26;
        }
        else
        {
            putc(digit, outfile);
            continue;
        }

        if (result > upperLimit || result < lowerLimit)
            putc(result - sign * delta, outfile);
        else
            putc(result, outfile);

        j++;
    }
}
