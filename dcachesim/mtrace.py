import re
import sys

# 集成式匹配模式（同时处理颜色代码和提取关键信息）
pattern = re.compile(
    r'.*?_([rw]ead|write)\].*?addr:\s+(0x[\da-fA-F]+)', 
    re.IGNORECASE
)

for line in sys.stdin:
    # 三步清洗流程
    cleaned = re.sub(
        r'\x1b\[[\d;]*m',  # 移除所有ANSI颜色代码
        '', 
        line.strip()
    )
    
    # 精确匹配访存操作
    if match := pattern.search(cleaned):
        op_map = {'read': 'read', 'write': 'write', 'ead': 'read'}  # 处理大小写变体
        op_type = op_map[match.group(1).lower()]
        print(f"{match.group(2)}")