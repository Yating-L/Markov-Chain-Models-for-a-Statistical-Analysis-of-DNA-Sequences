# Markov-Chain-Models-for-a-Statistical-Analysis-of-DNA-Sequences
1. Project Description

  This project is to implement the estimator for the Mm model and reproduce the results in the "Applications: Palindromes in E. coli". The sequence is taken from four sequences (L10328,M87049, L19201, and U00006 in Genbank) and consisted of 384,243 bp on the right side of the origin of replication. The estimator calculated the z-scores for 6-words , which are used to identify overrepreseted and underrepreseted sequences.
2. Implememtation

  long double expect_prob(char* word)
  /*calculate word expect probability*/

  int overlap(char* word, int dist)
  /*check whether word self-overlaped in distance like dist  ∂(W; d)*/

  long double overlap_exp(char* word, int dist) 
  /*calculate µ(WdW)*/

  long double self_count_sum(char* word) 
  /*calculate sum of the third term of MM model equation*/

  long double variation(char* word)
  /*calculate the variance*/

  z-socore = (obs - exp) / sqrt(variance)

3. Run program

  Compile: gcc counting.c
  
  Usage: ./a.out -min X -max Y [-descseq|-ascseq|-desctot|-asctot|-desclength|-asclength|-ascoe|-desoe|-asczscore|-deszscore] filename [other filenames]
  
  Example: Run program for word length = 6, M1 to M4 model
  
  gcc counting.c

  ./a.out -min 6 -max 6 -asczscore -order 1 all_seq.txt > m1_asczscore.txt
	
  ./a.out -min 6 -max 6 -deszscore -order 1 all_seq.txt > m1_deszscore.txt

  ./a.out -min 6 -max 6 -asczscore -order 2 all_seq.txt > m2_asczscore.txt

  ./a.out -min 6 -max 6 -deszscore -order 2 all_seq.txt > m2_deszscore.txt

  ./a.out -min 6 -max 6 -asczscore -order 3 all_seq.txt > m3_asczscore.txt
	
  ./a.out -min 6 -max 6 -deszscore -order 3 all_seq.txt > m3_deszscore.txt

  ./a.out -min 6 -max 6 -asczscore -order 4 all_seq.txt > m4_asczscore.txt

  ./a.out -min 6 -max 6 -deszscore -order 4 all_seq.txt > m4_deszscore.txt

3. Files

  Input sequence file: all_seq.txt
  
  Result files: My results are in folder results/myresults
  
  The results of m1 to m4 model from RMES are in folder results/rmes
  
  By comparing my results with Rmes results, the overrepresented and underrepresented word lists are the same and their z-scores are very close. There are some slightly different, because I skipped alphabet other than a, t, c, g.

Reference:

  Schbath, S., Prum, B., &amp; de Turckheim, E. (1995). Exceptional motifs in different Markov chain models for a statistical analysis of DNA sequences. Journal of Computational Biology, 2(3), 417-437.
