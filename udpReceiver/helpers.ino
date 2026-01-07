String getParamValue(String packet, String paramName) {
  String key = "$" + paramName + "=";
  int keyIndex = packet.indexOf(key);

  if (keyIndex == -1) return "";

  int valueStart = keyIndex + key.length();
  int valueEnd = packet.indexOf('/', valueStart);

  if (valueEnd == -1) return "";

  return packet.substring(valueStart, valueEnd);
}
