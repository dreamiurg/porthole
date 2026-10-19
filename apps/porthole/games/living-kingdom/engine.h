#pragma once
#include "content.h"

namespace kingdom {
void newCampaign(Save& save, uint32_t seed, uint8_t profile);
const Scene* currentScene(const Save& save);
bool hasFact(const Save& save, uint8_t id);
bool hasSeen(const Save& save, uint16_t id);
bool hasDecision(const Save& save, uint16_t id);
int itemCount(const Save& save, uint8_t id);
bool meets(const Save& save, Requirements requirements);
bool allowed(const Save& save, const Choice& choice);
const char* requirementReason(const Save& save, const Choice& choice);
bool sceneEligible(const Save& save, const Scene& scene);
const Scene* selectScene(const Save& save, Location location);
bool visit(Save& save, Location location);
void rest(Save& save);
void advanceDays(Save& save, uint16_t days);
bool commit(Save& save, uint8_t choiceIndex);
void finishResult(Save& save);
bool applyEffects(Save& save, Effects effects);
}  // namespace kingdom
