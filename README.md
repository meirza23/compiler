# 🚀 Custom Compiler Project

Bu proje, özel bir programlama dili için geliştirilmiş, **Lexer**, **Parser**, **Semantik Analiz** ve **Code Generation** aşamalarını içeren kapsamlı bir derleyici projesidir. C benzeri bir sözdizimine sahip olan dil, kendine özgü kontrol yapıları ve operatörlerle zenginleştirilmiştir.

---

## 📋 İçindekiler

1.  [Syntax ve Lexer (Sözcük Analizi)](#1-syntax-ve-lexer-sözcük-analizi)
2.  [Grammar ve Parser (Sözdizimi Analizi)](#2-grammar-ve-parser-sözdizimi-analizi)
3.  [Code Generation (Kod Üretimi)](#3-code-generation-kod-üretimi)
4.  [✨ Proje Özellikleri ve Test Senaryoları](#4--proje-özellikleri-ve-test-senaryoları)

---

## 1. Syntax ve Lexer (Sözcük Analizi)

Lexer aşaması, ham kaynak kodunu (string) anlamlı parçalara (token) ayırma işlemidir. Bu proje için **Flex** (Fast Lexical Analyzer Generator) kullanılmıştır.

### 🔍 Teknik Detaylar ve Regex Kuralları

Lexer, `lexer.l` dosyasında tanımlanan Düzenli İfadeler (Regular Expressions) ile çalışır. Aşağıda dilin kabul ettiği temel tokenlar ve tanımlayıcı regex desenleri verilmiştir:

| Token Tipi        | Regex Deseni            | Açıklama                                      | Örnek            |
| :---------------- | :---------------------- | :-------------------------------------------- | :--------------- |
| `TOKEN_ID`        | `[a-zA-Z][a-zA-Z0-9_]*` | Harfle başlamalı, alfanümerik devam edebilir. | `var_1`, `count` |
| `TOKEN_NUM_INT`   | `[0-9]+`                | Sadece rakamlardan oluşur.                    | `123`, `0`       |
| `TOKEN_NUM_FLOAT` | `[0-9]+\.[0-9]+`        | Nokta ile ayrılmış ondalıklı sayılar.         | `3.14`, `0.01`   |
| `TOKEN_ASSIGN`    | `:=`                    | Atama operatörü.                              | `x := 5`         |
| `COMMENT`         | `#`                     | Satır sonuna kadar olan yorumlar.             | `# Yorum`        |

### ✅ Valid vs Invalid Input

- **Doğru (Valid):** `myVar_1`, `3.14`, `:=`
- **Hatalı (Invalid):** `1var` (Rakamla başlayamaz), `.5` (Tam sayı kısmı olmalı), `=` (Atama için `:=` kullanılmalı).

---

## 2. Grammar ve Parser (Sözdizimi Analizi)

Parser aşaması, Lexer'dan gelen token akışının dilin gramer kurallarına uyup uymadığını kontrol eder ve **Abstract Syntax Tree (AST)** oluşturur. Bu işlem için **Bison** (GNU Parser Generator) kullanılmıştır.

### 📐 Gramer Yapısı ve LALR(1)

Parser, **LALR(1)** (Look-Ahead Left-to-Right Rightmost derivation) algoritmasını kullanır. `parser.y` dosyasında tanımlanan BNF (Backus-Naur Form) benzeri kurallar şöyledir:

- **Program Yapısı:** Program, fonksiyon listesinden (`func_list`) oluşur.
- **Fonksiyon Tanımı:** `TYPE ID '(' PARAMS ')' BLOCK` formatındadır.
- **Blok Yapısı:** `{}` yerine `begin ... end` blokları kullanılır.

### ⚖️ Operatör Önceliği (Precedence)

Matematiksel işlemlerin doğru sırayla yapılması için operatör öncelikleri tanımlanmıştır (Yüksekten düşüğe):

1.  `^` (Sağdan sola birleşim - Right Associative)
2.  `*`, `/`, `%` (Soldan sağa - Left Associative)
3.  `+`, `-` (Soldan sağa - Left Associative)
4.  `>`, `<`, `==`, `!=` (Karşılaştırma)

### 🌳 AST (Abstract Syntax Tree) Üretimi

Her gramer kuralı eşleştiğinde, `ast.c` içindeki fonksiyonlar çağrılarak bellekte düğümler oluşturulur.

- **Struct Yapısı:**
  ```c
  typedef struct ASTNode {
      NodeType type;      // NODE_IF, NODE_ASSIGN, vb.
      DataType data_type; // TYPE_INT, TYPE_FLOAT
      struct ASTNode *left, *right, *next; // Çocuk düğümler
      // ... değer ve isim alanları
  } ASTNode;
  ```

---

## 3. Code Generation (Kod Üretimi)

Derleyicinin son aşaması, oluşturulan AST'yi gezerek (Traversal) hedef makine için çalıştırılabilir kod üretmektir.

### ⚙️ Hedef: Stack-Based Virtual Machine (VM)

Bu derleyici, kayıtçı (register) tabanlı değil, **Yığın (Stack)** tabanlı bir sanal makine için kod üretir.

- **Stack Boyutu:** 100 elemanlık sabit boyutlu yığın.
- **Hafıza Modeli:** Değişkenler `memory` dizisinde saklanır ve indeksleri sembol tablosundan (`symbol_table`) yönetilir.

### 📝 Kod Üretim Mantığı (Traversal)

`generate_code` fonksiyonu AST'yi **Depth-First (Derinlik Öncelikli)** gezer. Bu yöntem, "Postfix" (Sonsal) notasyona benzer bir kod üretimi sağlar.

**Örnek Dönüşüm:** `x := a + b`

1.  `a` değişkenini yığına yükle (`LOAD a`).
2.  `b` değişkenini yığına yükle (`LOAD b`).
3.  Toplama işlemini yap (`ADD_I`). (Yığındaki üst iki elemanı alır, toplar, sonucu yığına atar).
4.  Sonucu `x` değişkenine kaydet (`STORE x`).

### 💾 Çıktı Formatı (`output.vm`)

Üretilen kodlar `.vm` uzantılı bir dosyaya yazılır. Bu dosya, satır satır okunarak yorumlanabilir (interpreted) veya binary formata çevrilebilir.

- **Valid Output:** `PUSH_INT 5`, `ADD_I`, `JZ LABEL_1`
- **Invalid State:** Stack underflow (yığın boşken veri çekme) veya Type mismatch (runtime'da float ile int toplama - gerçi bu derleyicide semantik analizde engellenir).

### 🔍 Tam Kod Üretim Örneği

Aşağıdaki basit kodun nasıl dönüştüğünü inceleyelim:

**Kaynak Kod:**

```text
int x.
x := 10.
if (x > 5) begin
    print(x).
end
```

---

## 4. ✨ Proje Özellikleri ve Test Senaryoları

Bu derleyici, standart dillerden farklılaşan özelliklere sahiptir. Aşağıda bu özellikler ve çalışan test senaryoları sunulmuştur.

### 🔹 Özellik 1: `unless` Kontrol Yapısı

`if (!condition)` ifadesinin daha temiz halidir. Koşul **yanlış** olduğu sürece çalışır.

**Test Kodu:**

```text
int x.
x := 5.
unless (x == 10) begin
    print(x). # 10 degilse yazdir
end
```

**AST Çıktısı:**

```text
UNLESS
  | OP: ==
  |   | VAR: x
  |   | NUM_INT: 10
  | BLOCK
  |   | PRINT
  |   |   | VAR: x
```

**Bytecode (`output.vm`):**

```text
LOAD x
PUSH_INT 10
EQ_I
JNZ LABEL_SKIP_0  ; Eşitse (Doğruysa) atla
LOAD x
PRINT >> [Deger: 5]
LABEL_SKIP_0:
```

---

### 🔹 Özellik 2: Üs Alma Operatörü (`^`)

Doğrudan üs alma işlemi için `^` operatörü tanımlanmıştır.

**Test Kodu:**

```text
int a.
a := 2 ^ 3.
print(a).
```

**Bytecode (`output.vm`):**

```text
PUSH_INT 2
PUSH_INT 3
POW_I             ; 2^3 islemi
STORE a
LOAD a
PRINT >> [Deger: 8]
```

---

### 🔹 Özellik 3: `while` Döngüsü

Klasik döngü yapısı, etiketler (`LABEL`) ve zıplama (`JMP`) komutları ile VM üzerinde çalıştırılır.

**Test Kodu:**

```text
int i.
i := 0.
while (i < 3) begin
    print(i).
    i := i + 1.
end
```

**Bytecode (`output.vm`):**

```text
LABEL_START_0:
LOAD i
PUSH_INT 3
LT_I              ; i < 3 mü?
JZ LABEL_END_1    ; Hayırsa bitir
LOAD i
PRINT >> [Deger: 0]
...
JMP LABEL_START_0 ; Başa dön
LABEL_END_1:
```

---

### 🔹 Özellik 4: Blok Yapısı (`begin-end`)

Kod blokları `begin` ve `end` ile tanımlanır, bu da Pascal benzeri okunabilir bir yapı sunar.

**Test Kodu:**

```text
int x.
x := 10.
begin
    x := x + 5.
end
print(x).
```

**Bytecode (`output.vm`):**

```text
PUSH_INT 10
STORE x
LOAD x
PUSH_INT 5
ADD_I
STORE x
LOAD x
PRINT >> [Deger: 15]
```

---

### ⚠️ Hata Yönetimi (Error Handling)

Derleyici, hatalı durumlarda kullanıcıyı uyarır.

| Hata Tipi         | Örnek Kod                     | Derleyici Çıktısı                        |
| :---------------- | :---------------------------- | :--------------------------------------- |
| **Syntax Error**  | `x = 5.`                      | `Satir 1: Bilinmeyen karakter: =`        |
| **Type Mismatch** | `int x. float y. x := x + y.` | `HATA: Farkli tiplerle islem yapilamaz!` |
| **Scope Error**   | `print(z).`                   | `Hata: Tanimlanmamis degisken 'z'!`      |
