SELECT * FROM t_biocard_template_link bt
JOIN face_vectors fv  ON fv.id=bt.uid AND bt.tmp_type=17 AND fv.id=1
