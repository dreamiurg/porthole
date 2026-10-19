#!/usr/bin/env python3
"""Regression checks for authored-content validation, independent of rendering."""

import copy
import json
import unittest

from content import ROOT, Compiler, validate, validate_lock


class ContentValidation(unittest.TestCase):
    def setUp(self):
        self.data = json.loads((ROOT / "content/chapter-01.json").read_text())
        self.compiler = Compiler(self.data)

    def rejects_predicate(self, kind, identity, value=1, **extra):
        with self.assertRaises(ValueError):
            self.compiler.condition({"kind": kind, "id": identity, "value": value, **extra})

    def rejects_effect(self, kind, identity, value=1, **extra):
        with self.assertRaises(ValueError):
            self.compiler.effects([{"kind": kind, "id": identity, "value": value, **extra}])

    def test_current_graph_compiles(self):
        validate(self.data)
        self.assertIn("static const Content CHAPTER", self.compiler.compile())

    def test_every_numeric_registry_reference_is_checked(self):
        for kind in ("fact", "item", "quest", "skill", "relation", "promise", "event"):
            with self.subTest(kind=kind):
                self.rejects_predicate(kind, 127)
                self.rejects_predicate(kind, "127")
                self.rejects_predicate(kind, "missing.key")

    def test_scene_and_choice_references_exist(self):
        for kind, identity in (("seen", 127), ("seen", "127"), ("decision", 511), ("decision", "511")):
            self.rejects_predicate(kind, identity)
        self.assertIn("Kind::Seen,1", self.compiler.condition({"kind": "seen", "id": 1, "value": 1}))

    def test_variant_and_value_bounds(self):
        for identity, value in ((4, 0), (-1, 1), (0, 2), (3, 3)):
            self.rejects_predicate("variant", identity, value)
        for kind, value in (("fact", 2), ("quest", 6), ("promise", 4), ("outcome", 5), ("skill", 7), ("relation", -6)):
            self.rejects_predicate(kind, 0, value)

    def test_invalid_predicate_kinds_and_comparisons(self):
        self.rejects_predicate("schedule", 0)
        self.rejects_predicate("unknown", 0)
        self.rejects_predicate("fact", 0, op="gt")
        self.rejects_predicate("fact", 0, value=[1])

    def test_effect_kinds_references_and_values(self):
        for kind in ("seen", "decision", "variant", "any", "unknown"):
            self.rejects_effect(kind, 0)
        for kind in ("fact", "item", "quest", "skill", "relation", "promise", "schedule", "location"):
            self.rejects_effect(kind, 127)
        for kind, value in (("fact", 2), ("item", -100), ("quest", 6), ("promise", 4), ("day", -1), ("deed", -1), ("outcome", 5)):
            self.rejects_effect(kind, 0, value)

    def test_schedules_and_seeded_values(self):
        self.rejects_effect("schedule", 0, 0)
        self.rejects_effect("schedule", 0, -1)
        self.rejects_effect("schedule", 0, 2, variant=4)
        self.rejects_effect("schedule", 0, 2, variant=-2)
        self.rejects_effect("fact", 0, 1, variant=0)
        self.compiler.effects([{"kind": "schedule", "id": 0, "value": 2, "variant": 2}])
        self.assertIn("Kind::Schedule,0,2,2", self.compiler.lines[-1])

    def test_broken_links_and_duplicate_ids(self):
        self.data["scenes"][0]["choices"][0]["next"] = 127
        with self.assertRaises(ValueError):
            validate(self.data)
        self.data["scenes"][0]["choices"][0]["next"] = 0
        self.data["scenes"][1]["id"] = 1
        with self.assertRaises(ValueError):
            validate(self.data)

    def test_missing_location_and_large_prose(self):
        original = copy.deepcopy(self.data)
        self.data["scenes"] = [scene for scene in self.data["scenes"] if scene["location"] != "tower"]
        with self.assertRaises(ValueError):
            validate(self.data)
        self.data = original
        self.data["scenes"][0]["paragraphs"] = ["x" * 8192]
        with self.assertRaises(ValueError):
            validate(self.data)

    def test_registry_capacity_and_duplicates(self):
        self.data["facts"].append(self.data["facts"][0])
        with self.assertRaises(ValueError):
            Compiler(self.data)
        self.data["facts"] = [f"fact.{n}" for n in range(65)]
        with self.assertRaises(ValueError):
            Compiler(self.data)

    def test_persisted_choice_slots_cannot_be_reordered(self):
        previous = copy.deepcopy(self.compiler.maps)
        self.data["scenes"][0]["choices"].reverse()
        changed = Compiler(self.data).maps
        self.assertEqual(previous["choices"], changed["choices"])
        with self.assertRaisesRegex(ValueError, "choice_slots"):
            validate_lock(previous, changed)

    def test_new_choices_append_without_moving_saved_slots(self):
        previous = copy.deepcopy(self.compiler.maps)
        choice = copy.deepcopy(self.data["scenes"][0]["choices"][0])
        choice.update(id=14, slug="intro.future_append")
        self.data["scenes"][0]["choices"].append(choice)
        validate_lock(previous, Compiler(self.data).maps)


if __name__ == "__main__":
    unittest.main()
