# openDEM

Open source Discrete Element Modeling

## Building

This project requires cmake.

Navigate to root directory:

### Linux
```bash
mkdir build
cd build
cmake ../src
make
bin/odem-sim path/to/input_file.json path/to/results.db
```

### Windows
I'm not a doctor. Documentation [here](http://www.cmake.org/cmake/help/runningcmake.html).

### Dependencies
1. libsqlite3
2. libjson
3. gtk+3
4. cairo2

## Theory

Based off of kinetic particle theory.

### Particle Interations

#### Spring model

* k -> spring constant
* e_12 -> vector normal to one particle in direction of the other particle
* e_12 = (x_2 - x_1) / norm(x_2 - x_1)

* delta -> distance between particles
* delta = norm(x_2 - x_1) - (r_1 + r_2)
* delta < 0 means that we have a collision

* x_1, x_2 -> positions of the first and second particles
* r_1, r_2 -> radii of the first and second particles

* F_1 = k*delta*e_12
* F_2 = -F_1
| where F_1, F_2 -> force on first particle, force on second particle

#### Spring-dashpot model
* Dashpot term is analogous to coefficient of restitution
* 0 < eta < 1

* v_12 -> relative velocity
* v_12 = v_2 - v_1

* F_1 = (k*delta + gamma*(v_12*e_12))*e_12
* F_2 = -F_1
* gamma >= 0 b/c ln(eta) > 0

* f_loss = sqrt(pi^2 + ln(eta)^2)
* m_12 = m_1*m_2/(m_1+m_2)
* t_coll = f_loss * sqrt(m_12/k)

* gamma -> energy loss, allows the model to lose energy in a controlled way
* gamma = -2*m_12*ln(eta) / t_coll

#### Coulomb friction model
* F_f = mu * F_n
* F_f -> frictional force
* mu -> frictional coefficient
* F_n -> normal force

* v_r <= v_glide --> mu(v_r) = mu_stick + (mu_stick - mu_glide)*(v_r/v_glide - 2) * v_r/v_glide
* v_glide < v_r <= v_limit --> mu(v_r) = mu_glide
* v_r > v_limit --> mu(v_r) = mu_glide * (1+v_ratio) / (1+mu_ratio*v_ratio)

* v_r -> relative tangential velocity
* v_glide -> velocity to overcome static friction
* v_limit -> velocity threshold to high velocity
* mu_stick -> static coefficient of friction
* mu_glide -> kinematic coefficient of friction
* mu_limit -> high velocity coefficient of friction

* v_ratio = (v_r - v_limit)/slope_limit
* mu_ratio = mu_glide/mu_limit
