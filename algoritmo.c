#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_ITEMS 7
#define NUM_TRANSACTIONS 10
#define MAX_LINE_LEN 256
#define MAX_PAIRS 50
#define MAX_RULES 100

// Nomes dos itens para facilitar a impressão dos resultados.
const char* ITEM_NAMES[NUM_ITEMS] = {"leite", "cafe", "cerveja", "pao", "manteiga", "arroz", "feijao"};

// Estrutura para armazenar um par frequente e seu suporte
typedef struct {
    int item1_idx;
    int item2_idx;
    float support;
} FrequentPair;

// Estrutura para armazenar a regra de associação final
typedef struct {
    int antecedent_idx;
    int consequent_idx;
    float support;
    float confidence;
} AssociationRule;


int main() {
    // Matriz para guardar todas as transações. 1 = sim, 0 = nao.
    int transactions[NUM_TRANSACTIONS][NUM_ITEMS] = {0};

    FILE *fp = fopen("tabela.csv", "r");
    if (fp == NULL) {
        perror("ERRO: Nao foi possivel abrir o arquivo 'transacoes.csv'");
        return 1;
    }

    char line[MAX_LINE_LEN];
    // Pula a linha do cabeçalho (nomes dos itens)
    fgets(line, sizeof(line), fp);

    int transaction_idx = 0;
    while (fgets(line, sizeof(line), fp) && transaction_idx < NUM_TRANSACTIONS) {
        char *token;
        int item_idx = 0;
        
        token = strtok(line, ",\n");
        while (token != NULL && item_idx < NUM_ITEMS) {
            // Se o valor for "sim", marca 1 na nossa matriz
            if (strcmp(token, "sim") == 0) {
                transactions[transaction_idx][item_idx] = 1;
            }
            token = strtok(NULL, ",\n");
            item_idx++;
        }
        transaction_idx++;
    }
    fclose(fp);

    // 2. Pegar os valores minimos de suporte e confiança
    float min_support, min_confidence;
    printf("--- Algoritmo de Regras de Associacao (Pares) ---\n");
    printf("Digite o valor de suporte minimo (ex: 0.3 para 30%%): ");
    scanf("%f", &min_support);
    printf("Digite o valor de confianca minima (ex: 0.7 para 70%%): ");
    scanf("%f", &min_confidence);
    printf("\n");

    // 3. Calcula o suporte para cada item individualmente, e registra apenas os que atingiram o suporte minimo
    printf("--- PASSO 1: Encontrando Itens Frequentes (L1) ---\n");
    float item_supports[NUM_ITEMS] = {0.0};
    int l1_items[NUM_ITEMS]; // Armazena os ÍNDICES dos itens frequentes (tipo: leite seria 0)
    int l1_count = 0;

    for (int j = 0; j < NUM_ITEMS; j++) {
        int count = 0;
        for (int i = 0; i < NUM_TRANSACTIONS; i++) {
            if (transactions[i][j] == 1) {
                count++;
            }
        }
        item_supports[j] = (float)count / NUM_TRANSACTIONS;
        
        if (item_supports[j] >= min_support) {
            l1_items[l1_count] = j;
            l1_count++;
            printf("Item {%s} e frequente. (Suporte: %.2f)\n", ITEM_NAMES[j], item_supports[j]);
        }
    }
    printf("Total de itens frequentes (L1): %d\n\n", l1_count);

    // 4. Calcula o suporte dos pares de itens frequentes, e também garante que os suporte desses itens seja maior que o suporte minimo
    printf("--- PASSO 2: Encontrando Pares Frequentes (L2) ---\n");
    FrequentPair l2_pairs[MAX_PAIRS];
    int l2_count = 0;

    for (int i = 0; i < l1_count; i++) {
        for (int j = i + 1; j < l1_count; j++) {
            int item1 = l1_items[i];
            int item2 = l1_items[j];
            
            int pair_count = 0;
            // Calcula o suporte do par
            for (int t = 0; t < NUM_TRANSACTIONS; t++) {
                if (transactions[t][item1] == 1 && transactions[t][item2] == 1) {
                    pair_count++;
                }
            }

            float pair_support = (float)pair_count / NUM_TRANSACTIONS;

            // Se o par tiver suporte maior que o suporte minimo, guarda ele
            if (pair_support >= min_support) {
                l2_pairs[l2_count].item1_idx = item1;
                l2_pairs[l2_count].item2_idx = item2;
                l2_pairs[l2_count].support = pair_support;
                l2_count++;
                printf("Par {%s, %s} e frequente. (Suporte: %.2f)\n", ITEM_NAMES[item1], ITEM_NAMES[item2], pair_support);
            }
        }
    }
    printf("Total de pares frequentes (L2): %d\n\n", l2_count);

    // 5. As regras são criadas a partir dos pares que passaram no teste de suporte anteriormente
    // A regra é basicamente um par que passou pelo teste de suporte e confiança minimos
    AssociationRule final_rules[MAX_RULES];
    int rules_count = 0;

    for (int i = 0; i < l2_count; i++) {
        FrequentPair pair = l2_pairs[i];
        
        // Confiança(A -> B) = Suporte({A, B}) / Suporte(A)

        // Regra 1: item1 -> item2
        float confidence1 = pair.support / item_supports[pair.item1_idx];
        if (confidence1 >= min_confidence) {
            final_rules[rules_count].antecedent_idx = pair.item1_idx;
            final_rules[rules_count].consequent_idx = pair.item2_idx;
            final_rules[rules_count].support = pair.support;
            final_rules[rules_count].confidence = confidence1;
            rules_count++;
        }

        // Regra 2: item2 -> item1
        float confidence2 = pair.support / item_supports[pair.item2_idx];
         if (confidence2 >= min_confidence) {
            final_rules[rules_count].antecedent_idx = pair.item2_idx;
            final_rules[rules_count].consequent_idx = pair.item1_idx;
            final_rules[rules_count].support = pair.support;
            final_rules[rules_count].confidence = confidence2;
            rules_count++;
        }
    }

    printf("--- RESULTADO FINAL: Regras de Associacao Validas ---\n");
    printf("Suporte Minimo: %.0f%% | Confianca Minima: %.0f%%\n\n", min_support * 100, min_confidence * 100);

    if (rules_count == 0) {
        printf("Nenhuma regra de associacao foi encontrada com os criterios definidos.\n");
    } else {
        for (int i = 0; i < rules_count; i++) {
            AssociationRule rule = final_rules[i];
            printf("Regra: {%s} -> {%s}\n", ITEM_NAMES[rule.antecedent_idx], ITEM_NAMES[rule.consequent_idx]);
            printf("   - Suporte: %.0f%%\n", rule.support * 100);
            printf("   - Confianca: %.0f%%\n\n", rule.confidence * 100);
        }
    }

    return 0;
}