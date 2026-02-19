#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

/* Tensor veri tipleri */
typedef enum {
    TENSOR_FLOAT32,  //tensor hangi turde veri tutacak//
    TENSOR_FLOAT16,
    TENSOR_INT8
} TensorType;

/* Union örneði: float ve ayný bitleri hex olarak okumak için */
typedef union {
    float float_value;
    uint32_t raw_bits;           //ayný bellek ýký farklý sekýlde okuma//
} BitAccess;

/* Tensor yapýsý */
typedef struct {
    int row_count;     //satýr sutun ve týpý//
    int col_count;
    TensorType tensor_type;

    union {
        float *float32_data;
        uint16_t *float16_data;   //tensor verisi //
        int8_t *int8_data;
    } data;

    float scale;    //int8 için deger donusumunde kullanýlýyorlar//
    int zero_point;

} Tensor;


/* float32 -> float16 dönüþümü */
uint16_t convert_float32_to_float16(float value)
{
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));    //bellekteki býtlerý bits degýskenýne kopyalar//

    uint16_t sign = (bits >> 31) & 1;       //iþaret bitini alýyoruz//
    int16_t exponent = ((bits >> 23) & 0xFF) - 127 + 15;  //float16 ya uygunluk//
    uint16_t mantissa = (bits >> 13) & 0x3FF;     //mantissa kýsmý akýnýr //    

    if (exponent <= 0) exponent = 0;
    if (exponent >= 31) exponent = 31;    //exponeti float16 sýnýrlarýnda tutAR//
    

    return (sign << 15) | (exponent << 10) | mantissa;  //float16 olusturulur//
}

/* float16 -> float32 dönüþümü */
float convert_float16_to_float32(uint16_t value)
{
    uint16_t sign = (value >> 15) & 1;   //sign exponet mantissa//
    uint16_t exponent = (value >> 10) & 0x1F;
    uint16_t mantissa = value & 0x3FF;

    int32_t exponent32 = exponent - 15 + 127;  //float32ye uyarlanýyor//
    uint32_t bits = (sign << 31) | (exponent32 << 23) | (mantissa << 13);  //float32 bit dizilimi olusturuluyor//

    float result;
    memcpy(&result, &bits, sizeof(result));   //bitleri floata kopyalar ve dondurur//
    return result;            
}

/* Tensor oluþturma */
Tensor create_tensor(int rows, int cols, TensorType type, float scale, int zero_point)
{
    Tensor tensor;
    tensor.row_count = rows;
    tensor.col_count = cols;
    tensor.tensor_type = type;    //tensor ozellikleri//
    tensor.scale = scale;
    tensor.zero_point = zero_point;

    int total_elements = rows * cols; //toplam eleman sayýsý//

    if (type == TENSOR_FLOAT32)
        tensor.data.float32_data = malloc(sizeof(float) * total_elements);
    else if (type == TENSOR_FLOAT16) 
        tensor.data.float16_data = malloc(sizeof(uint16_t) * total_elements);   //bellek ayýrýyoruz//
    else
        tensor.data.int8_data = malloc(sizeof(int8_t) * total_elements);

    return tensor;    //tensor geri dondurluru//
}

/* Tensor bellek temizleme */
void destroy_tensor(Tensor *tensor)
{
    if (tensor->tensor_type == TENSOR_FLOAT32)
        free(tensor->data.float32_data);
    else if (tensor->tensor_type == TENSOR_FLOAT16)
        free(tensor->data.float16_data);
    else
        free(tensor->data.int8_data);
}

/* Tensor bellekte kaç byte yer kaplýyor */
size_t calculate_memory_usage(Tensor *tensor)
{
    int total_elements = tensor->row_count * tensor->col_count;  //eleman sayýsý bulma//

    if (tensor->tensor_type == TENSOR_FLOAT32) return total_elements * 4;
    if (tensor->tensor_type == TENSOR_FLOAT16) return total_elements * 2;
    return total_elements * 1;
}

/* Float deðeri INT8'e quantize eder */
int8_t quantize_float_to_int8(float value, float scale, int zero_point)
{
    float quantized_value = value / scale + zero_point;

    if (quantized_value > 127) quantized_value = 127;
    if (quantized_value < -128) quantized_value = -128;

    return (int8_t)round(quantized_value);   //degri yuvarlar ve ýnt8 olarak dondurur//
}

/* INT8 deðeri tekrar float yapar (dequantize) */
float dequantize_int8_to_float(int8_t value, float scale, int zero_point)
{
    return (value - zero_point) * scale;
}

/* Tensor ekrana yazdýrma */
void print_tensor(Tensor *tensor)
{
    for (int i = 0; i < tensor->row_count; i++)   //döngü//
    {
        for (int j = 0; j < tensor->col_count; j++)
        {
            int index = i * tensor->col_count + j;  //index hesaplýyoruz//
            float value;

            if (tensor->tensor_type == TENSOR_FLOAT32)  //direkt deger alýnýr//            
                value = tensor->data.float32_data[index];

            else if (tensor->tensor_type == TENSOR_FLOAT16)        //float16 float32ye cevrilir//
                value = convert_float16_to_float32(tensor->data.float16_data[index]);

            else
                value = dequantize_int8_to_float(tensor->data.int8_data[index], tensor->scale, tensor->zero_point);  //inti floata cevirir//

            printf("%.3f ", value);
        }
        printf("\n");
    }
}

/* Matris çarpýmý (A x B = result) */
void matrix_multiply(Tensor *matrixA, Tensor *matrixB, Tensor *result)
{
    for (int i = 0; i < matrixA->row_count; i++)
    {
        for (int j = 0; j < matrixB->col_count; j++)
        {
            float sum = 0;

            for (int k = 0; k < matrixA->col_count; k++)   //matris için iç dongu//
            {
                float valueA = (matrixA->tensor_type == TENSOR_FLOAT32)
                    ? matrixA->data.float32_data[i * matrixA->col_count + k]   //a matrisinden eleman alýnýr //
                    : dequantize_int8_to_float(matrixA->data.int8_data[i * matrixA->col_count + k],
                                              matrixA->scale, matrixA->zero_point);

                float valueB = (matrixB->tensor_type == TENSOR_FLOAT32)
                    ? matrixB->data.float32_data[k * matrixB->col_count + j]
                    : dequantize_int8_to_float(matrixB->data.int8_data[k * matrixB->col_count + j],
                                              matrixB->scale, matrixB->zero_point);

                sum += valueA * valueB; //carpým//
            }

            result->data.float32_data[i * result->col_count + j] = sum; //sonuc resulta yazýlýr//
        }
    }
}

int main()
{
    printf("=== TinyML Tensor System ===\n\n");

    /* Float32 Tensor oluþturma */
    Tensor float_tensor = create_tensor(2, 2, TENSOR_FLOAT32, 0.1f, 0);

    float_tensor.data.float32_data[0] = 1.0f;
    float_tensor.data.float32_data[1] = 2.0f;  //degere yazma//
    float_tensor.data.float32_data[2] = 3.0f;
    float_tensor.data.float32_data[3] = 4.0f;

    printf("Original Float32 Tensor:\n");
    print_tensor(&float_tensor);

    /* INT8 Quantized Tensor oluþturma */
    Tensor quantized_tensor = create_tensor(2, 2, TENSOR_INT8, 0.1f, 0);

    for (int i = 0; i < 4; i++)
        quantized_tensor.data.int8_data[i] =
            quantize_float_to_int8(float_tensor.data.float32_data[i],   // float tensorundeký degerleri alýr ve int8cevirir sonra guantýzed tensorune yazar//
                                   quantized_tensor.scale,
                                   quantized_tensor.zero_point);

    printf("\nQuantized Tensor (Dequantized View):\n");
    print_tensor(&quantized_tensor);

    /*  sonuc tensoru*/
    Tensor result_tensor = create_tensor(2, 2, TENSOR_FLOAT32, 0, 0); //sonuc için float32  tensor olusturur//

    matrix_multiply(&float_tensor, &float_tensor, &result_tensor);

    printf("\nMatrix Multiplication (A x A):\n");
    print_tensor(&result_tensor);

    /* Union örneði */
    BitAccess union_demo;
    union_demo.float_value = 0.75f;  //unýona float degder yaZILIR//

    printf("\nUnion Demo:\n");
    printf("Float Value: %f\n", union_demo.float_value);
    printf("Raw Hex Bits: 0x%X\n", union_demo.raw_bits);

    /* Bellek kullanýmý */
    printf("\nMemory Usage:\n");
    printf("FLOAT32 Tensor: %zu bytes\n", calculate_memory_usage(&float_tensor));
    printf("INT8 Tensor: %zu bytes\n", calculate_memory_usage(&quantized_tensor));

    destroy_tensor(&float_tensor);
    destroy_tensor(&quantized_tensor);
    destroy_tensor(&result_tensor);  // malloc ýle atrýlan belleklri serbest býrakýr//

    return 0;
}

