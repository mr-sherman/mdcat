# Rendering Test

## Tables

### 1. Basic alignment (left / center / right)

| Left     | Center   | Right   |
|:---------|:--------:|--------:|
| a        | b        | c       |
| longer   | mid      | 1000    |
| x        | yyyyyyyy | 7       |

### 2. Uneven column widths

| ID | Description                                  | OK |
|----|-----------------------------------------------|----|
| 1  | Short                                          | :white_check_mark: |
| 2  | A much longer description that stretches this column | :x: |

### 3. Inline formatting inside cells

| Feature      | Status      | Notes                        |
|--------------|-------------|-------------------------------|
| **Bold**     | *italic*    | `inline code` here            |
| [Link](https://example.com) | done | :rocket: shipped it |
| plain        | plain       | plain                         |

### 4. Empty cells

| Name  | Value | Comment |
|-------|-------|---------|
| foo   |       | none    |
|       | 42    |         |
| bar   | 7     | ok      |

### 5. No leading/trailing pipes

Col A | Col B | Col C
------|-------|------
1     | 2     | 3
four  | five  | six

### 6. Single character columns

|a|b|c|
|-|-|-|
|1|2|3|
|4|5|6|

## Code Blocks

```cpp
#include <iostream>

int main() {
    // Print a greeting
    std::string name = "world";
    int count = 3;
    for (int i = 0; i < count; ++i) {
        std::cout << "Hello, " << name << "!" << std::endl;
    }
    return 0;
}
```

```python
def greet(name="world", times=3):
    # Print a greeting `times` times
    for i in range(times):
        print(f"Hello, {name}!")
    return True
```

```bash
#!/usr/bin/env bash
set -euo pipefail
for i in 1 2 3; do
    echo "count: $i"
done
```

```json
{
  "name": "mdcat",
  "version": 1,
  "enabled": true,
  "notes": null
}
```

```text
No highlighting is applied to unrecognized or missing language tags -- this
block falls back to the original flat rendering.
```

## Strikethrough

~~strikethrough text~~ mixed with **bold**, *italic*, and `inline code`.

## Task Lists

- [ ] unchecked task
- [x] checked task
- [X] checked task (capital X)
- regular bullet item for comparison
