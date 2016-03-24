# Data formats used by Psi Engine
### Psi Mesh (.msh):
| Data           | Size [bytes]                     |
| -------------- | -------------------------------- |
| (V)ertex count | 8                                |
| (I)ndex count  | 8                                |
| Bounding box   | 24                               |
| \0             | 1                                |
| Vertices       | V * sizeof(psi_util::VertexData) |
| \0             | 1                                |
| Indices        | I * 4                            |
| \0             | 1                                |
