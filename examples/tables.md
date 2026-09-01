# Table Rendering Test

## 1. Basic alignment (left / center / right)

| Left     | Center   | Right   |
|:---------|:--------:|--------:|
| a        | b        | c       |
| longer   | mid      | 1000    |
| x        | yyyyyyyy | 7       |

## 2. Uneven column widths

| ID | Description                                  | OK |
|----|-----------------------------------------------|----|
| 1  | Short                                          | :white_check_mark: |
| 2  | A much longer description that stretches this column | :x: |

## 3. Inline formatting inside cells

| Feature      | Status      | Notes                        |
|--------------|-------------|-------------------------------|
| **Bold**     | *italic*    | `inline code` here            |
| [Link](https://example.com) | done | :rocket: shipped it |
| plain        | plain       | plain                         |

## 4. Empty cells

| Name  | Value | Comment |
|-------|-------|---------|
| foo   |       | none    |
|       | 42    |         |
| bar   | 7     | ok      |

## 5. No leading/trailing pipes

Col A | Col B | Col C
------|-------|------
1     | 2     | 3
four  | five  | six

## 6. Single character columns

|a|b|c|
|-|-|-|
|1|2|3|
|4|5|6|
