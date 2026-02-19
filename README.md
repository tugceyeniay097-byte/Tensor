# Tensor 
Bu proje, C dili kullanılarak geliştirilen temel bir TinyML tensor altyapısını göstermektedir. Kodun amacı; farklı veri tipleri ile tensor oluşturma, veri dönüşümü (float32 ↔ float16), quantization (float → int8), dequantization (int8 → float), matris çarpımı ve bellek kullanım analizi gibi işlemleri düşük seviyede, manuel olarak gerçekleştirmektir.

Programın temel veri yapısı Tensor struct’ıdır. Bu yapı içerisinde:

row_count ve col_count ile tensorun boyutu,

tensor_type ile veri tipi (TENSOR_FLOAT32, TENSOR_FLOAT16, TENSOR_INT8),

union yapısı ile veri dizisi,

quantization için scale ve zero_point parametreleri

tanımlanmıştır.

Union kullanımı sayesinde aynı tensor yapısı, veri tipine bağlı olarak farklı pointer türleriyle (float*, uint16_t*, int8_t*) çalışabilmektedir. Bellek tahsisi create_tensor() fonksiyonunda dinamik olarak malloc ile yapılmaktadır. Veri tipine göre ayrılan byte miktarı değişmektedir. Bellek temizleme işlemi destroy_tensor() fonksiyonu ile gerçekleştirilir.

Float32 ↔ Float16 Dönüşümü

convert_float32_to_float16() fonksiyonunda float değer önce memcpy ile 32-bit integer yapıya aktarılır. Daha sonra bit kaydırma işlemleri ile:

sign biti

exponent alanı

mantissa alanı

ayrıştırılır. Exponent bias değeri 127’den 15’e dönüştürülerek 16-bit formata uyarlanır. Ardından sign, exponent ve mantissa tekrar birleştirilerek 16-bit sonuç oluşturulur.

Ters işlem convert_float16_to_float32() fonksiyonunda yapılmaktadır. Bu fonksiyonda 16-bit değerden sign, exponent ve mantissa ayrıştırılır, bias tekrar 127’ye uyarlanır ve 32-bit IEEE 754 formatı oluşturulur. Bu bölüm tamamen bit seviyesinde çalışmaktadır ve kayan noktalı sayı temsilini göstermektedir.

Quantization ve Dequantization

Quantization işlemi quantize_float_to_int8() fonksiyonunda lineer dönüşüm mantığı ile uygulanmıştır:

quantized_value = value / scale + zero_point

Sonuç round() ile en yakın tam sayıya yuvarlanır ve int8 sınırları (-128, 127) arasında sınırlandırılır (clipping). Bu yöntem TinyML uygulamalarında model boyutunu küçültmek ve hesaplama maliyetini azaltmak için kullanılır.

Dequantization işlemi dequantize_int8_to_float() fonksiyonunda şu formülle yapılmaktadır:

float_value = (value - zero_point) * scale

Bu işlem sayesinde int8 formatındaki veri tekrar yaklaşık float değerine dönüştürülmektedir.

print_tensor() fonksiyonunda eğer tensor tipi int8 ise, değer ekrana yazdırılmadan önce otomatik olarak dequantize edilerek float formatta gösterilmektedir.

Matris Çarpımı (Matrix Multiplication)

matrix_multiply() fonksiyonu klasik üçlü döngü yapısı ile çalışmaktadır:

Dış döngü: satırlar (i)

Orta döngü: sütunlar (j)

İç döngü: çarpım toplamı için (k)

Eleman erişimi satır-major düzenine göre:

index = i * col_count + j

formülü ile yapılmaktadır.

Eğer tensor tipi int8 ise, çarpım işlemi öncesinde değerler dequantize edilerek float’a dönüştürülmektedir. Çarpım işlemi float olarak yapılır ve sonuç tensoru float32 tipinde saklanır.

Union Kullanımı

BitAccess union yapısı ile aynı bellek alanı hem float hem de uint32_t olarak okunmaktadır. Bu sayede bir float değerin hem normal sayısal karşılığı hem de ham bit karşılığı (hex formatında) görüntülenmektedir. Bu bölüm, veri tiplerinin bellekte nasıl temsil edildiğini göstermek amacıyla eklenmiştir.

Bellek Kullanımı Analizi

calculate_memory_usage() fonksiyonu ile tensorun veri tipine göre kapladığı bellek miktarı hesaplanmaktadır:

float32 → 4 byte

float16 → 2 byte

int8 → 1 byte

Toplam eleman sayısı ile çarpılarak toplam bellek kullanımı elde edilir. Program sonunda float32 ve int8 tensorların bellek kullanımı karşılaştırılmaktadır.

Genel Amaç

Bu proje; TinyML sistemlerinde tensor yönetimi, veri temsili, quantization mantığı, bit seviyesinde dönüşüm ve bellek optimizasyonu konularını temel C dili kullanarak göstermek amacıyla geliştirilmiştir. Kod, harici bir kütüphane kullanmadan manuel olarak düşük seviyede işlem yapacak şekilde tasarlanmıştır.