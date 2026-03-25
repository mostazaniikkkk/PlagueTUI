import unittest
from conftest import load, free, rule_count


class TestParserBasic(unittest.TestCase):

    def test_empty_stylesheet(self):
        h = load("")
        self.assertEqual(rule_count(h), 0)
        free(h)

    def test_whitespace_only(self):
        h = load("   \n\t  ")
        self.assertEqual(rule_count(h), 0)
        free(h)

    def test_type_rule(self):
        h = load("Button { color: white; }")
        self.assertEqual(rule_count(h), 1)
        free(h)

    def test_class_rule(self):
        h = load(".my-class { background: black; }")
        self.assertEqual(rule_count(h), 1)
        free(h)

    def test_id_rule(self):
        h = load("#my-id { width: 20; }")
        self.assertEqual(rule_count(h), 1)
        free(h)

    def test_pseudo_rule(self):
        h = load("Button:hover { background: red; }")
        self.assertEqual(rule_count(h), 1)
        free(h)

    def test_multiple_rules(self):
        tcss = """
            Button  { color: white; }
            .panel  { background: darkblue; }
            #footer { height: 1; }
        """
        h = load(tcss)
        self.assertEqual(rule_count(h), 3)
        free(h)

    def test_comment_ignored(self):
        h = load("/* This is a comment */ Button { color: white; }")
        self.assertEqual(rule_count(h), 1)
        free(h)

    def test_comment_between_rules(self):
        h = load("Button { color: white; } /* sep */ Label { color: red; }")
        self.assertEqual(rule_count(h), 2)
        free(h)

    def test_multiple_declarations(self):
        tcss = "Button { color: white; background: black; width: 20; }"
        h = load(tcss)
        self.assertEqual(rule_count(h), 1)
        free(h)

    def test_universal_selector(self):
        h = load("* { color: white; }")
        self.assertEqual(rule_count(h), 1)
        free(h)

    def test_compound_type_class(self):
        h = load("Button.primary { color: blue; }")
        self.assertEqual(rule_count(h), 1)
        free(h)

    def test_variable_value_skipped(self):
        """Declaraciones con $variable en el valor deben ignorarse."""
        tcss = "Screen { background: $background; color: white; }"
        h = load(tcss)
        # La regla se parsea pero la declaración con $ se omite
        self.assertEqual(rule_count(h), 1)
        free(h)

    def test_handle_nonzero(self):
        h = load("Button { color: white; }")
        self.assertGreater(h, 0)
        free(h)

    def test_multiple_handles_independent(self):
        h1 = load("Button { color: white; }")
        h2 = load("Label { color: red; } Screen { background: black; }")
        self.assertEqual(rule_count(h1), 1)
        self.assertEqual(rule_count(h2), 2)
        free(h1)
        free(h2)


if __name__ == "__main__":
    unittest.main()
