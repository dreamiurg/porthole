// Pets Club's Ukrainian sentences (ukrainian.h, the design page "Sentence builder", approved 2026-11-22): a sentence in
// one language, its translation built from tiles. Steps 1-3 show Ukrainian and build English, steps 4-5 the other way,
// with wrong forms of the same Ukrainian words as decoys. host/test_pets_uk.cpp checks every one fits the round screen.
//   tiles:  the translation in order, tiles split by '|' (a step 1-2 tile can be a few words, from step 3 one word);
//   decoys: tiles that are in no translation;
//   alt:    other orders of the same tiles that are right too, as tile numbers, split by '|' ("70123456").
// en is the English sentence: telemetry names a sentence by it, so keep it stable.
#pragma once
#include <stdint.h>

namespace learn::uk {
struct Sentence { const char* uk; const char* en; uint8_t step, toUk; const char* tiles; const char* decoys; const char* alt; };

inline constexpr Sentence SENTENCES[] = {
  // step 1: three or four tiles, a decoy
  {"Це мій кіт.", "This is my cat.", 1, 0, "this is|my|cat", "dog", ""},
  {"Це моя мама.", "This is my mom.", 1, 0, "this is|my|mom", "dad", ""},
  {"Я люблю тата.", "I love dad.", 1, 0, "I|love|dad", "cat", ""},
  {"Собака біжить.", "The dog runs.", 1, 0, "the dog|runs", "sleeps", ""},
  {"Кіт спить.", "The cat is sleeping.", 1, 0, "the cat|is sleeping", "is eating", ""},
  {"Я їм яблуко.", "I eat an apple.", 1, 0, "I|eat|an apple", "drink", ""},
  {"Мама читає книжку.", "Mom reads a book.", 1, 0, "mom|reads|a book", "writes", ""},
  {"Тато пʼє чай.", "Dad drinks tea.", 1, 0, "dad|drinks|tea", "milk", ""},
  {"У мене є пес.", "I have a dog.", 1, 0, "I|have|a dog", "a cat", ""},
  {"Небо синє.", "The sky is blue.", 1, 0, "the sky|is|blue", "green", ""},
  {"Сонце світить.", "The sun is shining.", 1, 0, "the sun|is shining", "the moon", ""},
  {"Я люблю морозиво.", "I love ice cream.", 1, 0, "I|love|ice cream", "soup", ""},
  // step 2: three or four tiles, two decoys
  {"Сьогодні ми з татом підемо до школи.", "Today dad and I will go to school.", 2, 0, "today|dad and I|will go|to school", "home|mom", "1230"},
  {"Моя сестра читає книжку.", "My sister is reading a book.", 2, 0, "my sister|is reading|a book", "my brother|is writing", ""},
  {"Ми граємо у футбол після школи.", "We play football after school.", 2, 0, "we|play|football|after school", "basketball|at home", "3012"},
  {"Бабуся пече пиріг.", "Grandma is baking a pie.", 2, 0, "grandma|is baking|a pie", "grandpa|a soup", ""},
  {"Кіт сидить на дивані.", "The cat is sitting on the sofa.", 2, 0, "the cat|is sitting|on the sofa", "the dog|is eating", ""},
  {"Я йду до школи.", "I am going to school.", 2, 0, "I|am going|to school", "to the park|are going", ""},
  {"Тато працює вдома.", "Dad works at home.", 2, 0, "dad|works|at home", "mom|sleeps", ""},
  {"Діти гуляють у парку.", "The children walk in the park.", 2, 0, "the children|walk|in the park", "at school|sleep", ""},
  {"У нас є маленький пес.", "We have a little dog.", 2, 0, "we|have|a little dog", "a big dog|they", ""},
  {"Мій брат любить машинки.", "My brother loves toy cars.", 2, 0, "my brother|loves|toy cars", "my sister|books", ""},
  {"Взимку падає сніг.", "Snow falls in winter.", 2, 0, "snow|falls|in winter", "rain|in summer", "201"},
  {"Я малюю сонце.", "I am drawing the sun.", 2, 0, "I|am drawing|the sun", "the moon|am eating", ""},
  // step 3: a tile a word, two or three decoys
  {"Сьогодні ми з татом підемо до школи.", "Today dad and I will go to school.", 3, 0, "today|dad|and|I|will|go|to|school", "mom", "12345670"},
  {"Мама готує смачний суп.", "Mom is cooking tasty soup.", 3, 0, "mom|is|cooking|tasty|soup", "dad|cake|eating", ""},
  {"Мій друг живе біля школи.", "My friend lives near the school.", 3, 0, "my|friend|lives|near|the|school", "sister|far", ""},
  {"Ми їдемо до бабусі в суботу.", "We go to grandma's on Saturday.", 3, 0, "we|go|to|grandma's|on|Saturday", "mom|Sunday", "450123"},
  {"Влітку ми їздимо на море.", "In summer we go to the sea.", 3, 0, "in|summer|we|go|to|the|sea", "sun|swim", "2345601"},
  {"Кіт любить спати на сонці.", "The cat likes to sleep in the sun.", 3, 0, "the|cat|likes|to|sleep|in|the|sun", "dog", ""},
  {"Мій брат грає з собакою.", "My brother plays with the dog.", 3, 0, "my|brother|plays|with|the|dog", "sister|cat", ""},
  {"Я вмію рахувати до десяти.", "I can count to ten.", 3, 0, "I|can|count|to|ten", "read|five", ""},
  {"Восени листя стає жовтим.", "In autumn the leaves turn yellow.", 3, 0, "in|autumn|the|leaves|turn|yellow", "green|spring", "234501"},
  {"Мені подобається читати книжки.", "I like to read books.", 3, 0, "I|like|to|read|books", "write|toys", ""},
  {"Наш учитель дуже добрий.", "Our teacher is very kind.", 3, 0, "our|teacher|is|very|kind", "angry|my", ""},
  {"Після уроків ми граємо надворі.", "After school we play outside.", 3, 0, "after|school|we|play|outside", "inside|before", "23401"},
  // step 4: English to Ukrainian, wrong forms of the same words as decoys
  {"Моя сестра читає книжку.", "My sister is reading a book.", 4, 1, "моя|сестра|читає|книжку", "книжка|читаю", ""},
  {"Це мій кіт.", "This is my cat.", 4, 1, "це|мій|кіт", "моя|кота", ""},
  {"Мама готує суп.", "Mom is cooking soup.", 4, 1, "мама|готує|суп", "мамо|готую", ""},
  {"Тато пʼє чай.", "Dad drinks tea.", 4, 1, "тато|пʼє|чай", "тата|пʼю", ""},
  {"Я люблю свого собаку.", "I love my dog.", 4, 1, "я|люблю|свого|собаку", "собака|любить", ""},
  {"Ми йдемо до школи.", "We are going to school.", 4, 1, "ми|йдемо|до|школи", "школа|йду", ""},
  {"Кіт спить на дивані.", "The cat is sleeping on the sofa.", 4, 1, "кіт|спить|на|дивані", "диван|сплю", ""},
  {"Бабуся пече пиріг.", "Grandma is baking a pie.", 4, 1, "бабуся|пече|пиріг", "бабусю|печу", ""},
  {"Діти граються надворі.", "The children are playing outside.", 4, 1, "діти|граються|надворі", "дитина|граюся", ""},
  {"У мене є молодший брат.", "I have a little brother.", 4, 1, "у|мене|є|молодший|брат", "брата|мені", ""},
  {"Мій друг живе тут.", "My friend lives here.", 4, 1, "мій|друг|живе|тут", "друга|живу", ""},
  {"Сьогодні світить сонце.", "Today the sun is shining.", 4, 1, "сьогодні|світить|сонце", "сонця|світять", "021"},
  // step 5: longer, one to three decoys
  {"Сьогодні ми з татом підемо до школи.", "Today dad and I will go to school.", 5, 1, "сьогодні|ми|з|татом|підемо|до|школи", "тато", "1230456|1234560"},
  {"Влітку ми їздимо на море.", "In summer we go to the sea.", 5, 1, "влітку|ми|їздимо|на|море", "моря|їжджу|зима", "10234"},
  {"Мій брат грається з собакою.", "My brother plays with the dog.", 5, 1, "мій|брат|грається|з|собакою", "собака|граюся", ""},
  {"Бабуся живе у маленькому селі.", "Grandma lives in a small village.", 5, 1, "бабуся|живе|у|маленькому|селі", "село|живу", ""},
  {"У суботу ми їдемо до бабусі.", "We go to grandma's on Saturday.", 5, 1, "у|суботу|ми|їдемо|до|бабусі", "субота|бабуся|їду", "234501"},
  {"Кіт любить спати на сонці.", "The cat likes to sleep in the sun.", 5, 1, "кіт|любить|спати|на|сонці", "сонце|спить", ""},
  {"Я люблю читати книжки.", "I like to read books.", 5, 1, "я|люблю|читати|книжки", "читає|книжка", ""},
  {"Наш учитель дуже добрий.", "Our teacher is very kind.", 5, 1, "наш|учитель|дуже|добрий", "наша|добра", ""},
  {"Восени листя падає з дерев.", "In autumn the leaves fall from the trees.", 5, 1, "восени|листя|падає|з|дерев", "дерево|лист", ""},
  {"Після уроків ми граємо надворі.", "After school we play outside.", 5, 1, "після|уроків|ми|граємо|надворі", "урок|граю", "23401"},
  {"Моя мама працює у лікарні.", "My mom works in a hospital.", 5, 1, "моя|мама|працює|у|лікарні", "лікарня|працюю", ""},
  {"Діти малюють великий будинок.", "The children are drawing a big house.", 5, 1, "діти|малюють|великий|будинок", "малює|сад", ""},
};
constexpr int SENTENCE_COUNT = sizeof SENTENCES / sizeof *SENTENCES;
constexpr int SENTENCE_STEPS = 5;
}  // namespace learn::uk
