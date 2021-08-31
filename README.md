# Face Recognition with C++

First step is detecting the face. 

<img src="/resources/DeteckFace.jpg?raw=true"/>

---

Second step is feature extraction. I use local binary pattern for feature extraction.

<img src="/resources/LBP.jpg?raw=true"/>

---

Thrid step is dividing image some parts and save histogram of all parts.

<img src="/resources/hist.jpg?raw=true"/>

---

Last step is compering the new image with saved features. I used knn in this project for classification and error equal rate for threshold value calculation.
