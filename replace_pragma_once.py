#!/usr/bin/env python3
import os
import re

def generate_guard_name(filepath):
    """根据文件路径生成头文件保护宏名称"""
    # 从 mods 目录开始提取相对路径
    if '/mods/' in filepath:
        rel_path = filepath.split('/mods/')[1]
    else:
        rel_path = os.path.basename(filepath)
    
    # 将路径转换为大写的宏名称
    guard = rel_path.replace('/', '_').replace('.', '_').upper()
    return f"SERIKA_{guard}"

def replace_pragma_once(filepath):
    """替换单个文件中的 #pragma once"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"读取文件失败 {filepath}: {e}")
        return False
    
    # 如果已经使用 ifndef，跳过
    if re.search(r'#ifndef\s+\w+', content):
        print(f"跳过 (已使用 ifndef): {filepath}")
        return False
    
    # 如果没有 pragma once，跳过
    if '#pragma once' not in content:
        print(f"跳过 (无 pragma once): {filepath}")
        return False
    
    guard_name = generate_guard_name(filepath)
    
    # 替换 #pragma once (可能有尾随空格)
    content = re.sub(r'#pragma\s+once\s*', f'#ifndef {guard_name}\n#define {guard_name}\n', content, count=1)
    
    # 确保文件末尾有换行，然后添加 #endif
    content = content.rstrip()
    if not content.endswith('\n'):
        content += '\n'
    content += f'\n#endif // {guard_name}\n'
    
    try:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"✓ 已处理: {filepath}")
        return True
    except Exception as e:
        print(f"写入文件失败 {filepath}: {e}")
        return False

def main():
    """处理 mods 目录下的所有头文件"""
    project_dir = '/home/inory/UGit/Serika-Renderer-OpenGL'
    mods_dir = os.path.join(project_dir, 'mods')
    
    if not os.path.exists(mods_dir):
        print(f"错误: 找不到 mods 目录: {mods_dir}")
        return
    
    count = 0
    skipped = 0
    
    for root, dirs, files in os.walk(mods_dir):
        for file in files:
            if file.endswith('.h') or file.endswith('.hpp'):
                filepath = os.path.join(root, file)
                if replace_pragma_once(filepath):
                    count += 1
                else:
                    skipped += 1
    
    print(f"\n{'='*60}")
    print(f"处理完成！")
    print(f"成功替换: {count} 个文件")
    print(f"跳过: {skipped} 个文件")
    print(f"{'='*60}")

if __name__ == '__main__':
    main()
