import contextlib
import gzip
import importlib.util
import io
from pathlib import Path
import re
import tempfile
import unittest
from unittest.mock import patch


ROOT = Path(__file__).resolve().parent.parent


def load_generator(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / 'scripts' / f'{name}.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class HtmlGenerationTests(unittest.TestCase):
    def test_generators_ignore_time_and_checkout_line_endings(self):
        payloads = []
        for name in ('html_to_header', 'html_to_header_pio'):
            with self.subTest(generator=name), tempfile.TemporaryDirectory() as temp:
                generator = load_generator(name)
                html = Path(temp) / 'dashboard.html'
                header = Path(temp) / 'dashboard_html.h'
                hash_path = Path(temp) / '.dashboard_hash'
                outputs = []
                for now, content in ((1000, b'<p>Test</p>\n'), (2000, b'<p>Test</p>\r\n')):
                    html.write_bytes(content)
                    args = (html, header, hash_path) if name.endswith('_pio') else (str(html), str(header))
                    with patch('time.time', return_value=now), patch.object(generator, 'datetime', create=True) as clock:
                        clock.now.return_value.strftime.return_value = f'run-{now}'
                        with contextlib.redirect_stdout(io.StringIO()):
                            generator.generate_header(*args)
                    outputs.append(header.read_bytes())
                self.assertEqual(outputs[0], outputs[1])
                self.assertNotIn(b'\r\n', outputs[0])
                array = outputs[0].split(b'PROGMEM = {', 1)[1].split(b'};', 1)[0]
                compressed = bytes(int(value, 16) for value in re.findall(rb'0x([0-9A-F]{2})', array))
                self.assertEqual(compressed[4:8], b'\0' * 4)
                self.assertEqual(gzip.decompress(compressed), b'<p>Test</p>\n')
                payloads.append(compressed)
        self.assertEqual(payloads[0], payloads[1])

    def test_pio_invalidates_old_generation_cache(self):
        generator = load_generator('html_to_header_pio')
        with tempfile.TemporaryDirectory() as temp:
            html = Path(temp) / 'dashboard.html'
            header = Path(temp) / 'dashboard_html.h'
            hash_path = Path(temp) / '.dashboard_hash'
            content = b'<p>Test</p>\n'
            html.write_bytes(content)
            header.write_text('old header', encoding='utf-8')
            hash_path.write_text(generator.calculate_hash(content), encoding='utf-8')
            self.assertTrue(generator.should_regenerate(html, hash_path, header))
            with contextlib.redirect_stdout(io.StringIO()):
                generator.generate_header(html, header, hash_path)
            self.assertFalse(generator.should_regenerate(html, hash_path, header))
            html.write_bytes(content.replace(b'\n', b'\r\n'))
            self.assertFalse(generator.should_regenerate(html, hash_path, header))
            html.write_bytes(b'<p>Changed</p>\n')
            self.assertTrue(generator.should_regenerate(html, hash_path, header))


if __name__ == '__main__':
    unittest.main()
