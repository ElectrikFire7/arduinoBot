String getParam(String q, String key) {
  int i = q.indexOf(key + "=");
  if (i < 0) return "";
  int s = i + key.length() + 1;
  int e = q.indexOf("&", s);
  if (e < 0) e = q.length();
  return q.substring(s, e);
}

String urlDecode(String s) {
  String r = "";
  for (int i = 0; i < s.length(); i++) {
    if (s[i] == '+') r += ' ';
    else if (s[i] == '%' && i + 2 < s.length()) {
      r += char(strtol(s.substring(i + 1, i + 3).c_str(), NULL, 16));
      i += 2;
    } else r += s[i];
  }
  return r;
}