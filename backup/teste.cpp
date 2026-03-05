// #include <bits/stdc++.h>

// using namespace std;

// void solve(){
//     int n; cin >> n;
//     vector<string> s(n);
//     for (int i = 0; i < n; i++){
//         cin >> s[i];
//         reverse(s[i].begin(), s[i].end());
//     }


//     for (int j = 0; j < n; j++){
//         int ans = 0;
//         for (int i = 0; i < (int)s[j].size(); i++){
//             if (s[j][i] == '1'){
//                 ans += (1 << i);
//             }
//         }
//         cout << ans << " ";
//     }
//     cout << "\n";
// }

// int main(){
//     solve();
// }