
class Lagrange:
    _coefficients = None
    _x_values = None

    def __init__(self, pts_x: list, pts_y: list):
        if not (len(pts_x) == len(pts_y)):
            ValueError("len(pts_x) must equal len(pts_y)")

        self._coefficients = []
        self._x_values = pts_x
        N: int = len(pts_x)

        for idx in range(0, N):
            denom_product = 1
            cur = pts_x[idx]

            for jdx in range(0, N):
                if (jdx != idx):
                    denom_product *= cur - pts_x[jdx]

            self._coefficients.append(pts_y[idx] * (1 / denom_product))
            self._x_values.append(cur)
    
    def compute(self, x: float):
        out: float = 0.0
        for i in range(0, len(self._x_values)):
            cur_x = self._x_values[i]
            cur_coef = self._coefficients[i]
            prod = 1

            for x_val in self._x_values:
                if x_val != cur_x:
                    prod *= x - x_val
            out += prod * cur_coef
        return out