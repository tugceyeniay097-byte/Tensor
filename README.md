# Tensor 
Bu proje, C dilinde yazılmış basit bir TinyML tensor sistemini örneklemek amacıyla hazırlanmıştır. Programda 2 boyutlu bir tensor (matris) yapısı tanımlanmış ve bu yapı üzerinden farklı veri tipleriyle çalışma mantığı gösterilmiştir. Tensor yapısı içerisinde float32, float16 ve int8 veri tipleri desteklenmektedir. Böylece aynı verinin farklı hassasiyet ve bellek kullanımıyla nasıl saklanabileceği gösterilmiştir.

Kod içerisinde float32 değerlerin float16’ya dönüştürülmesi ve tekrar float32’ye çevrilmesi için dönüşüm fonksiyonları yazılmıştır. Ayrıca quantization işlemi uygulanarak float değerler int8 formatına çevrilmiş, ardından dequantization ile tekrar float değere yakın bir sonuç elde edilmiştir. Bu kısım özellikle TinyML sistemlerinde bellek tasarrufu ve performans açısından neden quantization kullanıldığını göstermek amacıyla eklenmiştir.

Projede ayrıca matris çarpımı (matrix multiplication) işlemi gerçekleştirilmiş ve sonuç tensoru hesaplanmıştır. Bunun yanında union yapısı kullanılarak bir float sayının hem normal değeri hem de bellekteki ham bit karşılığı (hex formatında) gösterilmiştir. Son olarak, farklı veri tiplerinin bellekte kaç byte yer kapladığı hesaplanarak karşılaştırma yapılmıştır.

Genel olarak bu çalışma, TinyML’de tensor mantığını, veri tipi dönüşümlerini, quantization kavramını ve temel matris işlemlerini daha iyi anlamak için hazırlanmıştır.
