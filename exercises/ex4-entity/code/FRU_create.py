#build a 256 bytes virtual EEPROM
fru_data = bytearray([0] * 256)

#[1]FRU Header (8 Bytes)
# Version: 0x01
# Internal Use Off: 0, Chassis Off: 0, Board Off: 1 (8 bytes), Product Off: 0, Multi Off: 0
# Checksum: 0xfe (1 + 1 + 254 = 256 mod 256 = 0)
header = [0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00]
header_cksum = (256 - (sum(header) % 256)) % 256
header.append(header_cksum)
fru_data[0:8] = header

# [2] Board Info Area (start from offset 8, 8 bytes long)
board_area = bytearray()
# Format Version: 0x01
board_area.append(0x01)
# Area Length: (8 Bytes) total: multiple of 8 bytes
# Header(2) + Language(1) + Date(3) + Mfg(1) + Name(1+len) + End(1)
# Language: 0 (English)
# Date: 0x00, 0x00, 0x00
# Manufacturer Type/Length: 0x00 (Empty)
temp_body = bytearray([0x00, 0x00, 0x00, 0x00, 0x00]) #
name = "Henry_Virtual_Card"
name_bytes = name.encode('utf-8')
temp_body.append(0xc0 + len(name_bytes)) # Product Name Type/Length: 0xc0 represents ASCII + length byte of name)
temp_body += name_bytes
temp_body.append(0xc1) #End of area marker

# total length = 1 (Version) + 1 (Length Byte) + temp_body + 1 (Checksum)
total_len = len(temp_body) + 3
area_len_units = (total_len + 7) // 8
actual_total_bytes = area_len_units * 8

board_area.append(area_len_units)
board_area += temp_body

# Padding : fill in 0 until checksum byte
padding_len = actual_total_bytes - len(board_area) - 1
board_area += bytearray([0] * padding_len)

# Calculate Board Area's Checksum
area_sum = (256 - (sum(board_area) % 256)) % 256
board_area.append(area_sum)

fru_data[8:8+len(board_area)] = board_area

with open("henry_fru.bin", "wb") as f:
    f.write(fru_data)
