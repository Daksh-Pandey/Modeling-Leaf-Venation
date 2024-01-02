# Modeling-Leaf-Venation
![Screenshot from 2023-12-13 11-53-34](https://github.com/Daksh-Pandey/Modeling-Leaf-Venation/assets/108348551/3a5c2a43-f0a2-49aa-89a8-89492964268f)
![Screenshot from 2023-12-13 14-39-25](https://github.com/Daksh-Pandey/Modeling-Leaf-Venation/assets/108348551/43378ced-bb6c-4de4-8e5c-5ba6b99791f0)

### Tech-stack:
* OpenGL
* ImGui
* C++

The aim of this project is to try to model different venation patterns in a variety of plant leaves. The algorithm to do so has been described in [Adams, 2005](https://dl.acm.org/doi/10.1145/1073204.1073251). I will also describe the different algorithms I used in an uncomplicated way here:-
* I described the margin/boundary of the leaf using [Gielis formula](https://en.wikipedia.org/wiki/Superformula). I found it an awesome way to mathematically describe the curves in nature.
* Next, I modeled the leaf growth in two ways - growth in the leaf margin and growth in the surface. The nitty - gritty of the implementation can be found [here](https://dl.acm.org/doi/10.1145/1073204.1073251).
* The main algorithm to simulate leaf venation is the result of interplay between auxin sources (something that attracts vein growth towards itself), vein nodes (look at leaf veins as a sort of tree graph, then, vein nodes are the nodes of that tree graph) and leaf growth.
* First, I try to generate the auxin sources in an even distribution using poisson disk sampling. I used [this](https://github.com/thinks/poisson-disk-sampling) to do so.
* Next, veins grow towards their nearest auxin source neighbours.
* After this, the auxin sources that got too close to vein nodes are removed.
* Finally, the leaf margin grows.
This project was done by me as the course project for Computer Graphics at IIITD.
