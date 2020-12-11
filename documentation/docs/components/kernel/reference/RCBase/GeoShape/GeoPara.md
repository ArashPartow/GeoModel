
### GeoPara

```cpp
// === GeoPara ===

  // Constructor:
  GeoPara (double XHalfLength, double YHalfLength, double ZHalfLength,
           double Alpha, double Theta, double Phi)

  // Public Methods:
  double getXHalfLength() const
  double getYHalfLength() const
  double getZHalfLength() const
  double getTheta() const
  double getAlpha() const
  double getPhi() const
```

The `GeoPara` class represents a parallelepiped. Faces at $\pm z$ are parallel to the $x-y$ plane. One edge of each of these faces is parallel to the $x$-axis, while the other edge makes an angle $\alpha$ with respect to the $y$-axis.  The remaining edge of the parallelepiped is oriented along a vector whose polar angle is $\theta$ and whose azimuthal angle is $\phi$.  *Half-lengths* in $x$, $y$, and $z$ describe the projections of the sides of the parallelepiped project onto the coordinate axes.  The constructor fills these data, while the accessors return them.

*Note:  Visualization of GeoPara is on the to-do list.*

<figure>
  <img src="https://dummyimage.com/600x400/eee/aaa" width="400" />
  <figcaption>Figure 4: GeoPara object, representing a parallelepiped.</figcaption>
</figure>

