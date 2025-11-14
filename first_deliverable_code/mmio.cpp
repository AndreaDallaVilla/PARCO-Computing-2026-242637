/* mmio.c  --  Matrix Market I/O library
 *
 * Author: NIST, National Institute of Standards and Technology
 * Date:   October 1993
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "mmio.h"

int mm_read_banner(FILE *f, MM_typecode *matcode)
{
    char line[MM_MAX_LINE_LENGTH];
    char banner[MM_MAX_LINE_LENGTH];
    char mtx[MM_MAX_LINE_LENGTH];
    char crd[MM_MAX_LINE_LENGTH];
    char data_type[MM_MAX_LINE_LENGTH];
    char storage_scheme[MM_MAX_LINE_LENGTH];
    char *p;

    mm_clear_typecode(matcode);

    if (fgets(line, MM_MAX_LINE_LENGTH, f) == NULL)
        return -1;

    if (sscanf(line, "%s %s %s %s %s", banner, mtx, crd, data_type, storage_scheme) != 5)
        return -1;

    for (p = mtx; *p != '\0'; *p = tolower(*p), p++);
    for (p = crd; *p != '\0'; *p = tolower(*p), p++);
    for (p = data_type; *p != '\0'; *p = tolower(*p), p++);
    for (p = storage_scheme; *p != '\0'; *p = tolower(*p), p++);

    if (strncmp(banner, MatrixMarketBanner, strlen(MatrixMarketBanner)) != 0)
        return -1;

    if (strcmp(mtx, MM_MTX_STR) != 0)
        return -1;
    mm_set_matrix(matcode);

    if (strcmp(crd, MM_COORDINATE_STR) == 0)
        mm_set_coordinate(matcode);
    else if (strcmp(crd, MM_ARRAY_STR) == 0)
        mm_set_array(matcode);
    else
        return -1;

    if (strcmp(data_type, MM_REAL_STR) == 0)
        mm_set_real(matcode);
    else if (strcmp(data_type, MM_COMPLEX_STR) == 0)
        mm_set_complex(matcode);
    else if (strcmp(data_type, MM_PATTERN_STR) == 0)
        mm_set_pattern(matcode);
    else if (strcmp(data_type, MM_INT_STR) == 0)
        mm_set_integer(matcode);
    else
        return -1;

    if (strcmp(storage_scheme, MM_GENERAL_STR) == 0)
        mm_set_general(matcode);
    else if (strcmp(storage_scheme, MM_SYMM_STR) == 0)
        mm_set_symmetric(matcode);
    else if (strcmp(storage_scheme, MM_HERM_STR) == 0)
        mm_set_hermitian(matcode);
    else if (strcmp(storage_scheme, MM_SKEW_STR) == 0)
        mm_set_skew(matcode);
    else
        return -1;

    return 0;
}

int mm_read_mtx_crd_size(FILE *f, int *M, int *N, int *nz)
{
    char line[MM_MAX_LINE_LENGTH];
    int num_items_read;

    do
    {
        if (fgets(line, MM_MAX_LINE_LENGTH, f) == NULL)
            return -1;
    } while (line[0] == '%');

    num_items_read = sscanf(line, "%d %d %d", M, N, nz);
    if (num_items_read != 3)
        return -1;
    return 0;
}

int mm_read_mtx_array_size(FILE *f, int *M, int *N)
{
    char line[MM_MAX_LINE_LENGTH];
    int num_items_read;

    do
    {
        if (fgets(line, MM_MAX_LINE_LENGTH, f) == NULL)
            return -1;
    } while (line[0] == '%');

    num_items_read = sscanf(line, "%d %d", M, N);
    if (num_items_read != 2)
        return -1;
    return 0;
}

int mm_write_banner(FILE *f, MM_typecode matcode)
{
    fprintf(f, "%s %s %s %s %s\n",
        MatrixMarketBanner,
        MM_MTX_STR,
        mm_is_sparse(matcode) ? MM_COORDINATE_STR : MM_ARRAY_STR,
        mm_is_real(matcode) ? MM_REAL_STR :
        mm_is_complex(matcode) ? MM_COMPLEX_STR :
        mm_is_pattern(matcode) ? MM_PATTERN_STR : MM_INT_STR,
        mm_is_general(matcode) ? MM_GENERAL_STR :
        mm_is_symmetric(matcode) ? MM_SYMM_STR :
        mm_is_hermitian(matcode) ? MM_HERM_STR : MM_SKEW_STR);

    return 0;
}

int mm_write_mtx_crd_size(FILE *f, int M, int N, int nz)
{
    fprintf(f, "%d %d %d\n", M, N, nz);
    return 0;
}

int mm_write_mtx_array_size(FILE *f, int M, int N)
{
    fprintf(f, "%d %d\n", M, N);
    return 0;
}

char *mm_typecode_to_str(MM_typecode matcode)
{
    static char buffer[MM_MAX_LINE_LENGTH];

    sprintf(buffer, "matrix %s %s %s",
        mm_is_sparse(matcode) ? MM_COORDINATE_STR : MM_ARRAY_STR,
        mm_is_real(matcode) ? MM_REAL_STR :
        mm_is_complex(matcode) ? MM_COMPLEX_STR :
        mm_is_pattern(matcode) ? MM_PATTERN_STR : MM_INT_STR,
        mm_is_general(matcode) ? MM_GENERAL_STR :
        mm_is_symmetric(matcode) ? MM_SYMM_STR :
        mm_is_hermitian(matcode) ? MM_HERM_STR : MM_SKEW_STR);

    return buffer;
}

