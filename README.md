# Tensor 

Bu proje, C dilinde yazılmış basit bir TinyML tensor örneğidir. Kodda float32, float16 ve int8 veri tipleri kullanılarak tensor oluşturma ve bellekte veri tutma mantığı gösterilmiştir. Ayrıca float değerlerin int8 formatına çevrilmesi (quantization) ve tekrar float’a dönüştürülmesi (dequantization) örnek olarak uygulanmıştır. Programın sonunda matris çarpımı işlemi yapılmış ve union yapısı kullanılarak bir float sayının bit düzeyinde nasıl saklandığı gösterilmiştir. Bu çalışma genel olarak tensor yapısını ve TinyML’de bellek/performans mantığını anlamak için hazırlanmıştır.

---